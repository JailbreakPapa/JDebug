#include <Foundation/FoundationPCH.h>

#include <Foundation/Application/Application.h>
#include <Foundation/Communication/DataTransfer.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/IdTable.h>
#include <Foundation/Containers/StaticRingBuffer.h>
#include <Foundation/IO/JSONWriter.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/ThreadUtils.h>

#if WD_ENABLED(WD_USE_PROFILING)

class wdProfileCaptureDataTransfer : public wdDataTransfer
{
private:
  virtual void OnTransferRequest() override
  {
    wdDataTransferObject dto(*this, "Capture", "application/json", "json");

    wdProfilingSystem::ProfilingData profilingData;
    wdProfilingSystem::Capture(profilingData);
    profilingData.Write(dto.GetWriter()).IgnoreResult();

    dto.Transmit();
  }
};

static wdProfileCaptureDataTransfer s_ProfileCaptureDataTransfer;

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, ProfilingSystem)

  // no dependencies

  ON_BASESYSTEMS_STARTUP
  {
    wdProfilingSystem::Initialize();
    s_ProfileCaptureDataTransfer.EnableDataTransfer("Profiling Capture");
  }
  ON_CORESYSTEMS_SHUTDOWN
  {
    s_ProfileCaptureDataTransfer.DisableDataTransfer();
    wdProfilingSystem::Reset();
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

namespace
{
  enum
  {
    BUFFER_SIZE_OTHER_THREAD = 1024 * 1024,
    BUFFER_SIZE_MAIN_THREAD = BUFFER_SIZE_OTHER_THREAD * 4 ///< Typically the main thread allocated a lot more profiling events than other threads
  };

  enum
  {
    BUFFER_SIZE_FRAMES = 120 * 60,
  };

  typedef wdStaticRingBuffer<wdProfilingSystem::GPUScope, BUFFER_SIZE_OTHER_THREAD / sizeof(wdProfilingSystem::GPUScope)> GPUScopesBuffer;

  static wdUInt64 s_MainThreadId = 0;

  struct CpuScopesBufferBase
  {
    virtual ~CpuScopesBufferBase() = default;

    wdUInt64 m_uiThreadId = 0;
    bool IsMainThread() const { return m_uiThreadId == s_MainThreadId; }
  };

  template <wdUInt32 SizeInBytes>
  struct CpuScopesBuffer : public CpuScopesBufferBase
  {
    wdStaticRingBuffer<wdProfilingSystem::CPUScope, SizeInBytes / sizeof(wdProfilingSystem::CPUScope)> m_Data;
  };

  CpuScopesBuffer<BUFFER_SIZE_MAIN_THREAD>* CastToMainThreadEventBuffer(CpuScopesBufferBase* pEventBuffer)
  {
    WD_ASSERT_DEV(pEventBuffer->IsMainThread(), "Implementation error");
    return static_cast<CpuScopesBuffer<BUFFER_SIZE_MAIN_THREAD>*>(pEventBuffer);
  }

  CpuScopesBuffer<BUFFER_SIZE_OTHER_THREAD>* CastToOtherThreadEventBuffer(CpuScopesBufferBase* pEventBuffer)
  {
    WD_ASSERT_DEV(!pEventBuffer->IsMainThread(), "Implementation error");
    return static_cast<CpuScopesBuffer<BUFFER_SIZE_OTHER_THREAD>*>(pEventBuffer);
  }

  wdCVarFloat cvar_ProfilingDiscardThresholdMS("Profiling.DiscardThresholdMS", 0.1f, wdCVarFlags::Default, "Discard profiling scopes if their duration is shorter than this in milliseconds.");

  wdStaticRingBuffer<wdTime, BUFFER_SIZE_FRAMES> s_FrameStartTimes;
  wdUInt64 s_uiFrameCount = 0;

  static wdHybridArray<wdProfilingSystem::ThreadInfo, 16> s_ThreadInfos;
  static wdHybridArray<wdUInt64, 16> s_DeadThreadIDs;
  static wdMutex s_ThreadInfosMutex;

#  if WD_ENABLED(WD_PLATFORM_64BIT)
  WD_CHECK_AT_COMPILETIME(sizeof(wdProfilingSystem::CPUScope) == 64);
  WD_CHECK_AT_COMPILETIME(sizeof(wdProfilingSystem::GPUScope) == 64);
#  endif

  static thread_local CpuScopesBufferBase* s_CpuScopes = nullptr;
  static wdDynamicArray<CpuScopesBufferBase*> s_AllCpuScopes;
  static wdMutex s_AllCpuScopesMutex;
  static wdProfilingSystem::ScopeTimeoutDelegate s_ScopeTimeoutCallback;

  static wdDynamicArray<wdUniquePtr<GPUScopesBuffer>> s_GPUScopes;

  static wdEventSubscriptionID s_PluginEventSubscription = 0;
  void PluginEvent(const wdPluginEvent& e)
  {
    if (e.m_EventType == wdPluginEvent::AfterUnloading)
    {
      // When a plugin is unloaded we need to clear all profiling data
      // since they can contain pointers to function names that don't exist anymore.
      wdProfilingSystem::Clear();
    }
  }
} // namespace

void wdProfilingSystem::ProfilingData::Clear()
{
  m_uiFramesThreadID = 0;
  m_uiProcessID = 0;
  m_uiFrameCount = 0;

  m_AllEventBuffers.Clear();
  m_FrameStartTimes.Clear();
  m_GPUScopes.Clear();
  m_ThreadInfos.Clear();
}

void wdProfilingSystem::ProfilingData::Merge(ProfilingData& out_merged, wdArrayPtr<const ProfilingData*> inputs)
{
  out_merged.Clear();

  if (inputs.IsEmpty())
    return;

  out_merged.m_uiProcessID = inputs[0]->m_uiProcessID;
  out_merged.m_uiFramesThreadID = inputs[0]->m_uiFramesThreadID;

  // concatenate m_FrameStartTimes and m_GPUScopes and m_uiFrameCount
  {
    wdUInt32 uiNumFrameStartTimes = 0;
    wdUInt32 uiNumGpuScopes = 0;

    for (const auto& pd : inputs)
    {
      out_merged.m_uiFrameCount += pd->m_uiFrameCount;

      uiNumFrameStartTimes += pd->m_FrameStartTimes.GetCount();
      uiNumGpuScopes += pd->m_GPUScopes.GetCount();
    }

    out_merged.m_FrameStartTimes.Reserve(uiNumFrameStartTimes);
    out_merged.m_GPUScopes.Reserve(uiNumGpuScopes);

    for (const auto& pd : inputs)
    {
      out_merged.m_FrameStartTimes.PushBackRange(pd->m_FrameStartTimes);
      out_merged.m_GPUScopes.PushBackRange(pd->m_GPUScopes);
    }
  }

  // merge m_ThreadInfos
  {
    auto threadInfoAlreadyKnown = [out_merged](wdUInt64 uiThreadId) -> bool {
      for (const auto& ti : out_merged.m_ThreadInfos)
      {
        if (ti.m_uiThreadId == uiThreadId)
          return true;
      }

      return false;
    };

    for (const auto& pd : inputs)
    {
      for (const auto& ti : pd->m_ThreadInfos)
      {
        if (!threadInfoAlreadyKnown(ti.m_uiThreadId))
        {
          out_merged.m_ThreadInfos.PushBack(ti);
        }
      }
    }
  }

  // merge m_AllEventBuffers
  {
    struct CountAndIndex
    {
      wdUInt32 m_uiCount = 0;
      wdUInt32 m_uiIndex = 0xFFFFFFFF;
    };

    wdMap<wdUInt64, CountAndIndex> eventBufferInfos;

    // gather info about required size of the output array
    for (const auto& pd : inputs)
    {
      for (const auto& eb : pd->m_AllEventBuffers)
      {
        auto& ebInfo = eventBufferInfos[eb.m_uiThreadId];

        ebInfo.m_uiIndex = wdMath::Min(ebInfo.m_uiIndex, eventBufferInfos.GetCount() - 1);
        ebInfo.m_uiCount += eb.m_Data.GetCount();
      }
    }

    // reserve the output array
    {
      out_merged.m_AllEventBuffers.SetCount(eventBufferInfos.GetCount());

      for (auto ebinfoIt : eventBufferInfos)
      {
        auto& neb = out_merged.m_AllEventBuffers[ebinfoIt.Value().m_uiIndex];
        neb.m_uiThreadId = ebinfoIt.Key();
        neb.m_Data.Reserve(ebinfoIt.Value().m_uiCount);
      }
    }

    // fill the output array
    for (const auto& pd : inputs)
    {
      for (const auto& eb : pd->m_AllEventBuffers)
      {
        const auto& ebInfo = eventBufferInfos[eb.m_uiThreadId];

        out_merged.m_AllEventBuffers[ebInfo.m_uiIndex].m_Data.PushBackRange(eb.m_Data);
      }
    }
  }
}

wdResult wdProfilingSystem::ProfilingData::Write(wdStreamWriter& ref_outputStream) const
{
  wdStandardJSONWriter writer;
  writer.SetWhitespaceMode(wdJSONWriter::WhitespaceMode::None);
  writer.SetOutputStream(&ref_outputStream);

  writer.BeginObject();
  {
    writer.BeginArray("traceEvents");

    // Process metadata
    {
      wdApplication::GetApplicationInstance()->GetApplicationName();

      writer.BeginObject();
      {
        writer.AddVariableString("name", "process_name");
        writer.AddVariableString("cat", "__metadata");
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableString("ph", "M");

        writer.BeginObject("args");
        writer.AddVariableString("name", wdApplication::GetApplicationInstance() ? wdApplication::GetApplicationInstance()->GetApplicationName().GetData() : "wdEngine");
        writer.EndObject();
      }
      writer.EndObject();

      writer.BeginObject();
      {
        writer.AddVariableString("name", "process_sort_index");
        writer.AddVariableString("cat", "__metadata");
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableString("ph", "M");

        writer.BeginObject("args");
        writer.AddVariableInt32("sort_index", m_uiProcessSortIndex);
        writer.EndObject();
      }
      writer.EndObject();
    }

    // Frames thread metadata
    {
      writer.BeginObject();
      {
        writer.AddVariableString("name", "thread_name");
        writer.AddVariableString("cat", "__metadata");
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", m_uiFramesThreadID);
        writer.AddVariableString("ph", "M");

        writer.BeginObject("args");
        writer.AddVariableString("name", "Frames");
        writer.EndObject();
      }
      writer.EndObject();

      writer.BeginObject();
      {
        writer.AddVariableString("name", "thread_sort_index");
        writer.AddVariableString("cat", "__metadata");
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", m_uiFramesThreadID);
        writer.AddVariableString("ph", "M");

        writer.BeginObject("args");
        writer.AddVariableInt32("sort_index", -1);
        writer.EndObject();
      }
      writer.EndObject();

      if (writer.HadWriteError())
      {
        return WD_FAILURE;
      }
    }

    const wdUInt32 uiGpuCount = m_GPUScopes.GetCount();
    // GPU thread metadata
    // Since there are no actual threads, we assign 1..uiGpuCount as the respective threadID
    for (wdUInt32 gpuIndex = 1; gpuIndex <= uiGpuCount; ++gpuIndex)
    {
      writer.BeginObject();
      {
        writer.AddVariableString("name", "thread_name");
        writer.AddVariableString("cat", "__metadata");
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", gpuIndex);
        writer.AddVariableString("ph", "M");

        wdStringBuilder gpuNameBuilder;
        gpuNameBuilder.AppendFormat("GPU {}", gpuIndex - 1);

        writer.BeginObject("args");
        writer.AddVariableString("name", gpuNameBuilder);
        writer.EndObject();
      }
      writer.EndObject();

      writer.BeginObject();
      {
        writer.AddVariableString("name", "thread_sort_index");
        writer.AddVariableString("cat", "__metadata");
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", gpuIndex);
        writer.AddVariableString("ph", "M");

        writer.BeginObject("args");
        writer.AddVariableInt32("sort_index", -2);
        writer.EndObject();
      }
      writer.EndObject();
      if (writer.HadWriteError())
      {
        return WD_FAILURE;
      }
    }

    // thread metadata
    {
      for (wdUInt32 threadIndex = 0; threadIndex < m_ThreadInfos.GetCount(); ++threadIndex)
      {
        const ThreadInfo& info = m_ThreadInfos[threadIndex];
        writer.BeginObject();
        {
          writer.AddVariableString("name", "thread_name");
          writer.AddVariableString("cat", "__metadata");
          writer.AddVariableUInt32("pid", m_uiProcessID);
          writer.AddVariableUInt64("tid", info.m_uiThreadId + uiGpuCount + 1);
          writer.AddVariableString("ph", "M");

          writer.BeginObject("args");
          writer.AddVariableString("name", info.m_sName);
          writer.EndObject();
        }
        writer.EndObject();

        writer.BeginObject();
        {
          writer.AddVariableString("name", "thread_sort_index");
          writer.AddVariableString("cat", "__metadata");
          writer.AddVariableUInt32("pid", m_uiProcessID);
          writer.AddVariableUInt64("tid", info.m_uiThreadId + uiGpuCount + 1);
          writer.AddVariableString("ph", "M");

          writer.BeginObject("args");
          writer.AddVariableInt32("sort_index", threadIndex);
          writer.EndObject();
        }
        writer.EndObject();

        if (writer.HadWriteError())
        {
          return WD_FAILURE;
        }
      }
    }

    // scoped events
    wdDynamicArray<CPUScope> sortedScopes;
    for (const auto& eventBuffer : m_AllEventBuffers)
    {
      // Since we introduced fake thread IDs via the GPUs, we simply shift all real thread IDs to be in a different range to avoid collisions.
      const wdUInt64 uiThreadId = eventBuffer.m_uiThreadId + uiGpuCount + 1;

      // It seems that chrome does a stable sort by scope begin time. Now that we write complete scopes at the end of a scope
      // we actually write nested scopes before their corresponding parent scope to the file. If both start at the same quantized time stamp
      // chrome prints the nested scope first and then scrambles everything.
      // So we sort by duration to make sure that parent scopes are written first in the json file.
      sortedScopes = eventBuffer.m_Data;
      sortedScopes.Sort([](const CPUScope& a, const CPUScope& b) { return (a.m_EndTime - a.m_BeginTime) > (b.m_EndTime - b.m_BeginTime); });

      for (const CPUScope& e : sortedScopes)
      {
        writer.BeginObject();
        writer.AddVariableString("name", e.m_szName);
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", uiThreadId);
        writer.AddVariableUInt64("ts", static_cast<wdUInt64>(e.m_BeginTime.GetMicroseconds()));
        writer.AddVariableString("ph", "B");

        if (e.m_szFunctionName != nullptr)
        {
          writer.BeginObject("args");
          writer.AddVariableString("function", e.m_szFunctionName);
          writer.EndObject();
        }

        writer.EndObject();

        if (e.m_EndTime.IsPositive())
        {
          writer.BeginObject();
          writer.AddVariableString("name", e.m_szName);
          writer.AddVariableUInt32("pid", m_uiProcessID);
          writer.AddVariableUInt64("tid", uiThreadId);
          writer.AddVariableUInt64("ts", static_cast<wdUInt64>(e.m_EndTime.GetMicroseconds()));
          writer.AddVariableString("ph", "E");
          writer.EndObject();
        }

        if (writer.HadWriteError())
        {
          return WD_FAILURE;
        }
      }
    }

    // frame start/end
    {
      wdStringBuilder sFrameName;

      const wdUInt32 uiNumFrames = m_FrameStartTimes.GetCount();
      for (wdUInt32 i = 1; i < uiNumFrames; ++i)
      {
        const wdTime t0 = m_FrameStartTimes[i - 1];
        const wdTime t1 = m_FrameStartTimes[i];

        const wdUInt64 localFrameID = uiNumFrames - i - 1;
        sFrameName.Format("Frame {}", m_uiFrameCount - localFrameID);

        writer.BeginObject();
        writer.AddVariableString("name", sFrameName);
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", m_uiFramesThreadID);
        writer.AddVariableUInt64("ts", static_cast<wdUInt64>(t0.GetMicroseconds()));
        writer.AddVariableString("ph", "B");
        writer.EndObject();

        writer.BeginObject();
        writer.AddVariableString("name", sFrameName);
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", m_uiFramesThreadID);
        writer.AddVariableUInt64("ts", static_cast<wdUInt64>(t1.GetMicroseconds()));
        writer.AddVariableString("ph", "E");
        writer.EndObject();
        if (writer.HadWriteError())
        {
          return WD_FAILURE;
        }
      }
    }

    // GPU data
    // Since there are no actual threads, we assign 1..gpuCount as the respective threadID
    {
      // See comment on sortedScopes above.
      wdDynamicArray<GPUScope> sortedGpuScopes;
      for (wdUInt32 gpuIndex = 1; gpuIndex <= m_GPUScopes.GetCount(); ++gpuIndex)
      {
        sortedGpuScopes = m_GPUScopes[gpuIndex - 1];
        sortedGpuScopes.Sort([](const GPUScope& a, const GPUScope& b) { return (a.m_EndTime - a.m_BeginTime) > (b.m_EndTime - b.m_BeginTime); });

        for (wdUInt32 i = 0; i < sortedGpuScopes.GetCount(); ++i)
        {
          const auto& e = sortedGpuScopes[i];

          writer.BeginObject();
          writer.AddVariableString("name", e.m_szName);
          writer.AddVariableUInt32("pid", m_uiProcessID);
          writer.AddVariableUInt64("tid", gpuIndex);
          writer.AddVariableUInt64("ts", static_cast<wdUInt64>(e.m_BeginTime.GetMicroseconds()));
          writer.AddVariableString("ph", "B");
          writer.EndObject();

          writer.BeginObject();
          writer.AddVariableString("name", e.m_szName);
          writer.AddVariableUInt32("pid", m_uiProcessID);
          writer.AddVariableUInt64("tid", gpuIndex);
          writer.AddVariableUInt64("ts", static_cast<wdUInt64>(e.m_EndTime.GetMicroseconds()));
          writer.AddVariableString("ph", "E");
          writer.EndObject();
          if (writer.HadWriteError())
          {
            return WD_FAILURE;
          }
        }
      }
    }

    writer.EndArray();
  }

  writer.EndObject();
  return writer.HadWriteError() ? WD_FAILURE : WD_SUCCESS;
}

// static
void wdProfilingSystem::Clear()
{
  {
    WD_LOCK(s_AllCpuScopesMutex);
    for (auto pEventBuffer : s_AllCpuScopes)
    {
      if (pEventBuffer->IsMainThread())
      {
        CastToMainThreadEventBuffer(pEventBuffer)->m_Data.Clear();
      }
      else
      {
        CastToOtherThreadEventBuffer(pEventBuffer)->m_Data.Clear();
      }
    }
  }

  s_FrameStartTimes.Clear();

  for (auto& gpuScopes : s_GPUScopes)
  {
    if (gpuScopes != nullptr)
    {
      gpuScopes->Clear();
    }
  }
}

// static
void wdProfilingSystem::Capture(wdProfilingSystem::ProfilingData& ref_profilingData, bool bClearAfterCapture)
{
  ref_profilingData.Clear();

  ref_profilingData.m_uiFramesThreadID = 0;
#  if WD_ENABLED(WD_SUPPORTS_PROCESSES)
  ref_profilingData.m_uiProcessID = wdProcess::GetCurrentProcessID();
#  else
  ref_profilingData.m_uiProcessID = 0;
#  endif

  {
    WD_LOCK(s_ThreadInfosMutex);

    if (bClearAfterCapture)
    {
      ref_profilingData.m_ThreadInfos = std::move(s_ThreadInfos);
    }
    else
    {
      ref_profilingData.m_ThreadInfos = s_ThreadInfos;
    }
  }

  {
    WD_LOCK(s_AllCpuScopesMutex);

    ref_profilingData.m_AllEventBuffers.Reserve(s_AllCpuScopes.GetCount());
    for (wdUInt32 i = 0; i < s_AllCpuScopes.GetCount(); ++i)
    {
      const auto& sourceEventBuffer = s_AllCpuScopes[i];
      CPUScopesBufferFlat& targetEventBuffer = ref_profilingData.m_AllEventBuffers.ExpandAndGetRef();

      targetEventBuffer.m_uiThreadId = sourceEventBuffer->m_uiThreadId;

      wdUInt32 uiSourceCount = sourceEventBuffer->IsMainThread() ? CastToMainThreadEventBuffer(sourceEventBuffer)->m_Data.GetCount() : CastToOtherThreadEventBuffer(sourceEventBuffer)->m_Data.GetCount();
      targetEventBuffer.m_Data.SetCountUninitialized(uiSourceCount);
      for (wdUInt32 j = 0; j < uiSourceCount; ++j)
      {
        const CPUScope& sourceEvent = sourceEventBuffer->IsMainThread() ? CastToMainThreadEventBuffer(sourceEventBuffer)->m_Data[j] : CastToOtherThreadEventBuffer(sourceEventBuffer)->m_Data[j];

        CPUScope& copiedEvent = targetEventBuffer.m_Data[j];
        copiedEvent.m_szFunctionName = sourceEvent.m_szFunctionName;
        copiedEvent.m_BeginTime = sourceEvent.m_BeginTime;
        copiedEvent.m_EndTime = sourceEvent.m_EndTime;
        wdStringUtils::Copy(copiedEvent.m_szName, CPUScope::NAME_SIZE, sourceEvent.m_szName);
      }
    }
  }

  ref_profilingData.m_uiFrameCount = s_uiFrameCount;

  ref_profilingData.m_FrameStartTimes.SetCountUninitialized(s_FrameStartTimes.GetCount());
  for (wdUInt32 i = 0; i < s_FrameStartTimes.GetCount(); ++i)
  {
    ref_profilingData.m_FrameStartTimes[i] = s_FrameStartTimes[i];
  }

  if (!s_GPUScopes.IsEmpty())
  {
    for (const auto& gpuScopes : s_GPUScopes)
    {
      if (gpuScopes != nullptr)
      {
        wdDynamicArray<GPUScope>& gpuScopesCopy = ref_profilingData.m_GPUScopes.ExpandAndGetRef();
        gpuScopesCopy.SetCountUninitialized(gpuScopes->GetCount());
        for (wdUInt32 i = 0; i < gpuScopes->GetCount(); ++i)
        {
          const GPUScope& sourceGpuDat = (*gpuScopes)[i];

          GPUScope& copiedGpuData = gpuScopesCopy[i];
          copiedGpuData.m_BeginTime = sourceGpuDat.m_BeginTime;
          copiedGpuData.m_EndTime = sourceGpuDat.m_EndTime;
          wdStringUtils::Copy(copiedGpuData.m_szName, GPUScope::NAME_SIZE, sourceGpuDat.m_szName);
        }
      }
    }
  }

  if (bClearAfterCapture)
  {
    Clear();
  }
}

// static
void wdProfilingSystem::SetDiscardThreshold(wdTime threshold)
{
  cvar_ProfilingDiscardThresholdMS = static_cast<float>(threshold.GetMilliseconds());
}

void wdProfilingSystem::SetScopeTimeoutCallback(ScopeTimeoutDelegate callback)
{
  s_ScopeTimeoutCallback = callback;
}

// static
wdUInt64 wdProfilingSystem::GetFrameCount()
{
  return s_uiFrameCount;
}

// static
void wdProfilingSystem::StartNewFrame()
{
  ++s_uiFrameCount;

  if (!s_FrameStartTimes.CanAppend())
  {
    s_FrameStartTimes.PopFront();
  }

  s_FrameStartTimes.PushBack(wdTime::Now());
}

// static
void wdProfilingSystem::AddCPUScope(wdStringView sName, const char* szFunctionName, wdTime beginTime, wdTime endTime, wdTime scopeTimeout)
{
  const wdTime duration = endTime - beginTime;

  // discard?
  if (duration < wdTime::Milliseconds(cvar_ProfilingDiscardThresholdMS))
    return;

  ::CpuScopesBufferBase* pScopes = s_CpuScopes;

  if (pScopes == nullptr)
  {
    if (wdThreadUtils::IsMainThread())
    {
      pScopes = WD_DEFAULT_NEW(::CpuScopesBuffer<BUFFER_SIZE_MAIN_THREAD>);
    }
    else
    {
      pScopes = WD_DEFAULT_NEW(::CpuScopesBuffer<BUFFER_SIZE_OTHER_THREAD>);
    }

    pScopes->m_uiThreadId = (wdUInt64)wdThreadUtils::GetCurrentThreadID();
    s_CpuScopes = pScopes;

    {
      WD_LOCK(s_AllCpuScopesMutex);
      s_AllCpuScopes.PushBack(pScopes);
    }
  }

  CPUScope scope;
  scope.m_szFunctionName = szFunctionName;
  scope.m_BeginTime = beginTime;
  scope.m_EndTime = endTime;
  wdStringUtils::Copy(scope.m_szName, WD_ARRAY_SIZE(scope.m_szName), sName.GetStartPointer(), sName.GetEndPointer());

  if (wdThreadUtils::IsMainThread())
  {
    auto pMainThreadBuffer = CastToMainThreadEventBuffer(pScopes);
    if (!pMainThreadBuffer->m_Data.CanAppend())
    {
      pMainThreadBuffer->m_Data.PopFront();
    }

    pMainThreadBuffer->m_Data.PushBack(scope);
  }
  else
  {
    auto pOtherThreadBuffer = CastToOtherThreadEventBuffer(pScopes);
    if (!pOtherThreadBuffer->m_Data.CanAppend())
    {
      pOtherThreadBuffer->m_Data.PopFront();
    }

    pOtherThreadBuffer->m_Data.PushBack(scope);
  }

  if (scopeTimeout.IsPositive() && duration > scopeTimeout && s_ScopeTimeoutCallback.IsValid())
  {
    s_ScopeTimeoutCallback(sName, szFunctionName, duration);
  }
}

// static
void wdProfilingSystem::Initialize()
{
  SetThreadName("Main Thread");
  s_MainThreadId = (wdUInt64)wdThreadUtils::GetCurrentThreadID();

  s_PluginEventSubscription = wdPlugin::Events().AddEventHandler(&PluginEvent);
}

// static
void wdProfilingSystem::Reset()
{
  WD_LOCK(s_ThreadInfosMutex);
  WD_LOCK(s_AllCpuScopesMutex);
  for (wdUInt32 i = 0; i < s_DeadThreadIDs.GetCount(); i++)
  {
    wdUInt64 uiThreadId = s_DeadThreadIDs[i];
    for (wdUInt32 k = 0; k < s_ThreadInfos.GetCount(); k++)
    {
      if (s_ThreadInfos[k].m_uiThreadId == uiThreadId)
      {
        // Don't use swap as a thread ID could be re-used and so we might delete the
        // info for an actual thread in the next loop instead of the remnants of the thread
        // that existed before.
        s_ThreadInfos.RemoveAtAndCopy(k);
        break;
      }
    }
    for (wdUInt32 k = 0; k < s_AllCpuScopes.GetCount(); k++)
    {
      CpuScopesBufferBase* pEventBuffer = s_AllCpuScopes[k];
      if (pEventBuffer->m_uiThreadId == uiThreadId)
      {
        WD_DEFAULT_DELETE(pEventBuffer);
        // Forward order and no swap important, see comment above.
        s_AllCpuScopes.RemoveAtAndCopy(k);
      }
    }
  }
  s_DeadThreadIDs.Clear();

  wdPlugin::Events().RemoveEventHandler(s_PluginEventSubscription);
}

// static
void wdProfilingSystem::SetThreadName(wdStringView sThreadName)
{
  WD_LOCK(s_ThreadInfosMutex);

  ThreadInfo& info = s_ThreadInfos.ExpandAndGetRef();
  info.m_uiThreadId = (wdUInt64)wdThreadUtils::GetCurrentThreadID();
  info.m_sName = sThreadName;
}

// static
void wdProfilingSystem::RemoveThread()
{
  WD_LOCK(s_ThreadInfosMutex);

  s_DeadThreadIDs.PushBack((wdUInt64)wdThreadUtils::GetCurrentThreadID());
}

// static
void wdProfilingSystem::InitializeGPUData(wdUInt32 uiGpuCount)
{
  if (s_GPUScopes.GetCount() < uiGpuCount)
  {
    s_GPUScopes.SetCount(uiGpuCount);
  }

  for (auto& gpuScopes : s_GPUScopes)
  {
    if (gpuScopes == nullptr)
    {
      gpuScopes = WD_DEFAULT_NEW(GPUScopesBuffer);
    }
  }
}

void wdProfilingSystem::AddGPUScope(wdStringView sName, wdTime beginTime, wdTime endTime, wdUInt32 uiGpuIndex)
{
  // discard?
  if (endTime - beginTime < wdTime::Milliseconds(cvar_ProfilingDiscardThresholdMS))
    return;

  if (!s_GPUScopes[uiGpuIndex]->CanAppend())
  {
    s_GPUScopes[uiGpuIndex]->PopFront();
  }

  GPUScope scope;
  scope.m_BeginTime = beginTime;
  scope.m_EndTime = endTime;
  wdStringUtils::Copy(scope.m_szName, WD_ARRAY_SIZE(scope.m_szName), sName.GetStartPointer(), sName.GetEndPointer());

  s_GPUScopes[uiGpuIndex]->PushBack(scope);
}

//////////////////////////////////////////////////////////////////////////

wdProfilingScope::wdProfilingScope(wdStringView sName, const char* szFunctionName, wdTime timeout)
  : m_sName(sName)
  , m_szFunction(szFunctionName)
  , m_BeginTime(wdTime::Now())
  , m_Timeout(timeout)
{
}

wdProfilingScope::~wdProfilingScope()
{
  wdProfilingSystem::AddCPUScope(m_sName, m_szFunction, m_BeginTime, wdTime::Now(), m_Timeout);
}

//////////////////////////////////////////////////////////////////////////

thread_local wdProfilingListScope* wdProfilingListScope::s_pCurrentList = nullptr;

wdProfilingListScope::wdProfilingListScope(wdStringView sListName, wdStringView sFirstSectionName, const char* szFunctionName)
  : m_sListName(sListName)
  , m_szListFunction(szFunctionName)
  , m_ListBeginTime(wdTime::Now())
  , m_sCurSectionName(sFirstSectionName)
  , m_CurSectionBeginTime(m_ListBeginTime)
{
  m_pPreviousList = s_pCurrentList;
  s_pCurrentList = this;
}

wdProfilingListScope::~wdProfilingListScope()
{
  wdTime now = wdTime::Now();
  wdProfilingSystem::AddCPUScope(m_sCurSectionName, nullptr, m_CurSectionBeginTime, now, wdTime::Zero());
  wdProfilingSystem::AddCPUScope(m_sListName, m_szListFunction, m_ListBeginTime, now, wdTime::Zero());

  s_pCurrentList = m_pPreviousList;
}

// static
void wdProfilingListScope::StartNextSection(wdStringView sNextSectionName)
{
  wdProfilingListScope* pCurScope = s_pCurrentList;

  wdTime now = wdTime::Now();
  wdProfilingSystem::AddCPUScope(pCurScope->m_sCurSectionName, nullptr, pCurScope->m_CurSectionBeginTime, now, wdTime::Zero());

  pCurScope->m_sCurSectionName = sNextSectionName;
  pCurScope->m_CurSectionBeginTime = now;
}

#else

wdResult wdProfilingSystem::ProfilingData::Write(wdStreamWriter& outputStream) const
{
  return WD_FAILURE;
}

void wdProfilingSystem::Clear() {}

void wdProfilingSystem::Capture(wdProfilingSystem::ProfilingData& out_Capture, bool bClearAfterCapture) {}

void wdProfilingSystem::SetDiscardThreshold(wdTime threshold) {}

void wdProfilingSystem::StartNewFrame() {}

void wdProfilingSystem::AddCPUScope(wdStringView sName, const char* szFunctionName, wdTime beginTime, wdTime endTime, wdTime scopeTimeout) {}

void wdProfilingSystem::Initialize() {}

void wdProfilingSystem::Reset() {}

void wdProfilingSystem::SetThreadName(wdStringView sThreadName) {}

void wdProfilingSystem::RemoveThread() {}

void wdProfilingSystem::InitializeGPUData(wdUInt32 gpuCount) {}

void wdProfilingSystem::AddGPUScope(wdStringView sName, wdTime beginTime, wdTime endTime, wdUInt32 gpuIndex) {}

void wdProfilingSystem::ProfilingData::Merge(ProfilingData& out_Merged, wdArrayPtr<const ProfilingData*> inputs) {}

#endif

WD_STATICLINK_FILE(Foundation, Foundation_Profiling_Implementation_Profiling);
