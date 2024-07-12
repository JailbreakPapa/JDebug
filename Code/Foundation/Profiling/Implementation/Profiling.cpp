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

#if NS_ENABLED(NS_USE_PROFILING)

class nsProfileCaptureDataTransfer : public nsDataTransfer
{
private:
  virtual void OnTransferRequest() override
  {
    nsDataTransferObject dto(*this, "Capture", "application/json", "json");

    nsProfilingSystem::ProfilingData profilingData;
    nsProfilingSystem::Capture(profilingData);
    profilingData.Write(dto.GetWriter()).IgnoreResult();

    dto.Transmit();
  }
};

static nsProfileCaptureDataTransfer s_ProfileCaptureDataTransfer;

namespace
{
  static nsEventSubscriptionID s_PluginEventSubscription = 0;
  void PluginEvent(const nsPluginEvent& e)
  {
    if (e.m_EventType == nsPluginEvent::AfterUnloading)
    {
      // When a plugin is unloaded we need to clear all profiling data
      // since they can contain pointers to function names that don't exist anymore.
      nsProfilingSystem::Clear();
    }
  }
} // namespace

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, ProfilingSystem)

  // no dependencies

  ON_BASESYSTEMS_STARTUP
  {
    nsProfilingSystem::Initialize();
  }
  ON_CORESYSTEMS_STARTUP
  { 
    s_PluginEventSubscription = nsPlugin::Events().AddEventHandler(&PluginEvent);
    s_ProfileCaptureDataTransfer.EnableDataTransfer("Profiling Capture");
  }
  ON_CORESYSTEMS_SHUTDOWN
  {
    s_ProfileCaptureDataTransfer.DisableDataTransfer();
    nsPlugin::Events().RemoveEventHandler(s_PluginEventSubscription);
    nsProfilingSystem::Reset();
  }

NS_END_SUBSYSTEM_DECLARATION;
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

  using GPUScopesBuffer = nsStaticRingBuffer<nsProfilingSystem::GPUScope, BUFFER_SIZE_OTHER_THREAD / sizeof(nsProfilingSystem::GPUScope)>;

  static nsUInt64 s_MainThreadId = 0;

  struct CpuScopesBufferBase
  {
    virtual ~CpuScopesBufferBase() = default;

    nsUInt64 m_uiThreadId = 0;
    bool IsMainThread() const { return m_uiThreadId == s_MainThreadId; }
  };

  template <nsUInt32 SizeInBytes>
  struct CpuScopesBuffer : public CpuScopesBufferBase
  {
    nsStaticRingBuffer<nsProfilingSystem::CPUScope, SizeInBytes / sizeof(nsProfilingSystem::CPUScope)> m_Data;
  };

  CpuScopesBuffer<BUFFER_SIZE_MAIN_THREAD>* CastToMainThreadEventBuffer(CpuScopesBufferBase* pEventBuffer)
  {
    NS_ASSERT_DEV(pEventBuffer->IsMainThread(), "Implementation error");
    return static_cast<CpuScopesBuffer<BUFFER_SIZE_MAIN_THREAD>*>(pEventBuffer);
  }

  CpuScopesBuffer<BUFFER_SIZE_OTHER_THREAD>* CastToOtherThreadEventBuffer(CpuScopesBufferBase* pEventBuffer)
  {
    NS_ASSERT_DEV(!pEventBuffer->IsMainThread(), "Implementation error");
    return static_cast<CpuScopesBuffer<BUFFER_SIZE_OTHER_THREAD>*>(pEventBuffer);
  }

  nsCVarFloat cvar_ProfilingDiscardThresholdMS("Profiling.DiscardThresholdMS", 0.1f, nsCVarFlags::Default, "Discard profiling scopes if their duration is shorter than this in milliseconds.");

  nsStaticRingBuffer<nsTime, BUFFER_SIZE_FRAMES> s_FrameStartTimes;
  nsUInt64 s_uiFrameCount = 0;

  static nsHybridArray<nsProfilingSystem::ThreadInfo, 16> s_ThreadInfos;
  static nsHybridArray<nsUInt64, 16> s_DeadThreadIDs;
  static nsMutex s_ThreadInfosMutex;

#  if NS_ENABLED(NS_PLATFORM_64BIT)
  NS_CHECK_AT_COMPILETIME(sizeof(nsProfilingSystem::CPUScope) == 64);
  NS_CHECK_AT_COMPILETIME(sizeof(nsProfilingSystem::GPUScope) == 64);
#  endif

  static thread_local CpuScopesBufferBase* s_CpuScopes = nullptr;
  static nsDynamicArray<CpuScopesBufferBase*> s_AllCpuScopes;
  static nsMutex s_AllCpuScopesMutex;
  static nsProfilingSystem::ScopeTimeoutDelegate s_ScopeTimeoutCallback;

  static nsDynamicArray<nsUniquePtr<GPUScopesBuffer>> s_GPUScopes;
} // namespace

void nsProfilingSystem::ProfilingData::Clear()
{
  m_uiFramesThreadID = 0;
  m_uiProcessID = 0;
  m_uiFrameCount = 0;

  m_AllEventBuffers.Clear();
  m_FrameStartTimes.Clear();
  m_GPUScopes.Clear();
  m_ThreadInfos.Clear();
}

void nsProfilingSystem::ProfilingData::Merge(ProfilingData& out_merged, nsArrayPtr<const ProfilingData*> inputs)
{
  out_merged.Clear();

  if (inputs.IsEmpty())
    return;

  out_merged.m_uiProcessID = inputs[0]->m_uiProcessID;
  out_merged.m_uiFramesThreadID = inputs[0]->m_uiFramesThreadID;

  // concatenate m_FrameStartTimes and m_GPUScopes and m_uiFrameCount
  {
    nsUInt32 uiNumFrameStartTimes = 0;
    nsUInt32 uiNumGpuScopes = 0;

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
    auto threadInfoAlreadyKnown = [out_merged](nsUInt64 uiThreadId) -> bool
    {
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
      nsUInt32 m_uiCount = 0;
      nsUInt32 m_uiIndex = 0xFFFFFFFF;
    };

    nsMap<nsUInt64, CountAndIndex> eventBufferInfos;

    // gather info about required size of the output array
    for (const auto& pd : inputs)
    {
      for (const auto& eb : pd->m_AllEventBuffers)
      {
        auto& ebInfo = eventBufferInfos[eb.m_uiThreadId];

        ebInfo.m_uiIndex = nsMath::Min(ebInfo.m_uiIndex, eventBufferInfos.GetCount() - 1);
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

nsResult nsProfilingSystem::ProfilingData::Write(nsStreamWriter& ref_outputStream) const
{
  nsStandardJSONWriter writer;
  writer.SetWhitespaceMode(nsJSONWriter::WhitespaceMode::None);
  writer.SetOutputStream(&ref_outputStream);

  writer.BeginObject();
  {
    writer.BeginArray("traceEvents");

    // Process metadata
    {
      nsApplication::GetApplicationInstance()->GetApplicationName();

      writer.BeginObject();
      {
        writer.AddVariableString("name", "process_name");
        writer.AddVariableString("cat", "__metadata");
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableString("ph", "M");

        writer.BeginObject("args");
        writer.AddVariableString("name", nsApplication::GetApplicationInstance() ? nsApplication::GetApplicationInstance()->GetApplicationName().GetData() : "nsEngine");
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
        return NS_FAILURE;
      }
    }

    const nsUInt32 uiGpuCount = m_GPUScopes.GetCount();
    // GPU thread metadata
    // Since there are no actual threads, we assign 1..uiGpuCount as the respective threadID
    for (nsUInt32 gpuIndex = 1; gpuIndex <= uiGpuCount; ++gpuIndex)
    {
      writer.BeginObject();
      {
        writer.AddVariableString("name", "thread_name");
        writer.AddVariableString("cat", "__metadata");
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", gpuIndex);
        writer.AddVariableString("ph", "M");

        nsStringBuilder gpuNameBuilder;
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
        return NS_FAILURE;
      }
    }

    // thread metadata
    {
      for (nsUInt32 threadIndex = 0; threadIndex < m_ThreadInfos.GetCount(); ++threadIndex)
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
          return NS_FAILURE;
        }
      }
    }

    // scoped events
    nsDynamicArray<CPUScope> sortedScopes;
    for (const auto& eventBuffer : m_AllEventBuffers)
    {
      // Since we introduced fake thread IDs via the GPUs, we simply shift all real thread IDs to be in a different range to avoid collisions.
      const nsUInt64 uiThreadId = eventBuffer.m_uiThreadId + uiGpuCount + 1;

      // It seems that chrome does a stable sort by scope begin time. Now that we write complete scopes at the end of a scope
      // we actually write nested scopes before their corresponding parent scope to the file. If both start at the same quantized time stamp
      // chrome prints the nested scope first and then scrambles everything.
      // So we sort by duration to make sure that parent scopes are written first in the json file.
      sortedScopes = eventBuffer.m_Data;
      sortedScopes.Sort([](const CPUScope& a, const CPUScope& b)
        { return (a.m_EndTime - a.m_BeginTime) > (b.m_EndTime - b.m_BeginTime); });

      for (const CPUScope& e : sortedScopes)
      {
        writer.BeginObject();
        writer.AddVariableString("name", static_cast<const char*>(e.m_szName));
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", uiThreadId);
        writer.AddVariableUInt64("ts", static_cast<nsUInt64>(e.m_BeginTime.GetMicroseconds()));
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
          writer.AddVariableString("name", static_cast<const char*>(e.m_szName));
          writer.AddVariableUInt32("pid", m_uiProcessID);
          writer.AddVariableUInt64("tid", uiThreadId);
          writer.AddVariableUInt64("ts", static_cast<nsUInt64>(e.m_EndTime.GetMicroseconds()));
          writer.AddVariableString("ph", "E");
          writer.EndObject();
        }

        if (writer.HadWriteError())
        {
          return NS_FAILURE;
        }
      }
    }

    // frame start/end
    {
      nsStringBuilder sFrameName;

      const nsUInt32 uiNumFrames = m_FrameStartTimes.GetCount();
      for (nsUInt32 i = 1; i < uiNumFrames; ++i)
      {
        const nsTime t0 = m_FrameStartTimes[i - 1];
        const nsTime t1 = m_FrameStartTimes[i];

        const nsUInt64 localFrameID = uiNumFrames - i - 1;
        sFrameName.SetFormat("Frame {}", m_uiFrameCount - localFrameID);

        writer.BeginObject();
        writer.AddVariableString("name", sFrameName);
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", m_uiFramesThreadID);
        writer.AddVariableUInt64("ts", static_cast<nsUInt64>(t0.GetMicroseconds()));
        writer.AddVariableString("ph", "B");
        writer.EndObject();

        writer.BeginObject();
        writer.AddVariableString("name", sFrameName);
        writer.AddVariableUInt32("pid", m_uiProcessID);
        writer.AddVariableUInt64("tid", m_uiFramesThreadID);
        writer.AddVariableUInt64("ts", static_cast<nsUInt64>(t1.GetMicroseconds()));
        writer.AddVariableString("ph", "E");
        writer.EndObject();
        if (writer.HadWriteError())
        {
          return NS_FAILURE;
        }
      }
    }

    // GPU data
    // Since there are no actual threads, we assign 1..gpuCount as the respective threadID
    {
      // See comment on sortedScopes above.
      nsDynamicArray<GPUScope> sortedGpuScopes;
      for (nsUInt32 gpuIndex = 1; gpuIndex <= m_GPUScopes.GetCount(); ++gpuIndex)
      {
        sortedGpuScopes = m_GPUScopes[gpuIndex - 1];
        sortedGpuScopes.Sort([](const GPUScope& a, const GPUScope& b)
          { return (a.m_EndTime - a.m_BeginTime) > (b.m_EndTime - b.m_BeginTime); });

        for (nsUInt32 i = 0; i < sortedGpuScopes.GetCount(); ++i)
        {
          const auto& e = sortedGpuScopes[i];

          writer.BeginObject();
          writer.AddVariableString("name", static_cast<const char*>(e.m_szName));
          writer.AddVariableUInt32("pid", m_uiProcessID);
          writer.AddVariableUInt64("tid", gpuIndex);
          writer.AddVariableUInt64("ts", static_cast<nsUInt64>(e.m_BeginTime.GetMicroseconds()));
          writer.AddVariableString("ph", "B");
          writer.EndObject();

          writer.BeginObject();
          writer.AddVariableString("name", static_cast<const char*>(e.m_szName));
          writer.AddVariableUInt32("pid", m_uiProcessID);
          writer.AddVariableUInt64("tid", gpuIndex);
          writer.AddVariableUInt64("ts", static_cast<nsUInt64>(e.m_EndTime.GetMicroseconds()));
          writer.AddVariableString("ph", "E");
          writer.EndObject();
          if (writer.HadWriteError())
          {
            return NS_FAILURE;
          }
        }
      }
    }

    writer.EndArray();
  }

  writer.EndObject();

  return writer.HadWriteError() ? NS_FAILURE : NS_SUCCESS;
}

// static
void nsProfilingSystem::Clear()
{
  {
    NS_LOCK(s_AllCpuScopesMutex);
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
void nsProfilingSystem::Capture(nsProfilingSystem::ProfilingData& ref_profilingData, bool bClearAfterCapture)
{
  ref_profilingData.Clear();

  ref_profilingData.m_uiFramesThreadID = 0;
#  if NS_ENABLED(NS_SUPPORTS_PROCESSES)
  ref_profilingData.m_uiProcessID = nsProcess::GetCurrentProcessID();
#  else
  ref_profilingData.m_uiProcessID = 0;
#  endif

  {
    NS_LOCK(s_ThreadInfosMutex);

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
    NS_LOCK(s_AllCpuScopesMutex);

    ref_profilingData.m_AllEventBuffers.Reserve(s_AllCpuScopes.GetCount());
    for (nsUInt32 i = 0; i < s_AllCpuScopes.GetCount(); ++i)
    {
      const auto& sourceEventBuffer = s_AllCpuScopes[i];
      CPUScopesBufferFlat& targetEventBuffer = ref_profilingData.m_AllEventBuffers.ExpandAndGetRef();

      targetEventBuffer.m_uiThreadId = sourceEventBuffer->m_uiThreadId;

      nsUInt32 uiSourceCount = sourceEventBuffer->IsMainThread() ? CastToMainThreadEventBuffer(sourceEventBuffer)->m_Data.GetCount() : CastToOtherThreadEventBuffer(sourceEventBuffer)->m_Data.GetCount();
      targetEventBuffer.m_Data.SetCountUninitialized(uiSourceCount);
      for (nsUInt32 j = 0; j < uiSourceCount; ++j)
      {
        const CPUScope& sourceEvent = sourceEventBuffer->IsMainThread() ? CastToMainThreadEventBuffer(sourceEventBuffer)->m_Data[j] : CastToOtherThreadEventBuffer(sourceEventBuffer)->m_Data[j];

        CPUScope& copiedEvent = targetEventBuffer.m_Data[j];
        copiedEvent.m_szFunctionName = sourceEvent.m_szFunctionName;
        copiedEvent.m_BeginTime = sourceEvent.m_BeginTime;
        copiedEvent.m_EndTime = sourceEvent.m_EndTime;
        nsStringUtils::Copy(copiedEvent.m_szName, CPUScope::NAME_SIZE, sourceEvent.m_szName);
      }
    }
  }

  ref_profilingData.m_uiFrameCount = s_uiFrameCount;

  ref_profilingData.m_FrameStartTimes.SetCountUninitialized(s_FrameStartTimes.GetCount());
  for (nsUInt32 i = 0; i < s_FrameStartTimes.GetCount(); ++i)
  {
    ref_profilingData.m_FrameStartTimes[i] = s_FrameStartTimes[i];
  }

  if (!s_GPUScopes.IsEmpty())
  {
    for (const auto& gpuScopes : s_GPUScopes)
    {
      if (gpuScopes != nullptr)
      {
        nsDynamicArray<GPUScope>& gpuScopesCopy = ref_profilingData.m_GPUScopes.ExpandAndGetRef();
        gpuScopesCopy.SetCountUninitialized(gpuScopes->GetCount());
        for (nsUInt32 i = 0; i < gpuScopes->GetCount(); ++i)
        {
          const GPUScope& sourceGpuDat = (*gpuScopes)[i];

          GPUScope& copiedGpuData = gpuScopesCopy[i];
          copiedGpuData.m_BeginTime = sourceGpuDat.m_BeginTime;
          copiedGpuData.m_EndTime = sourceGpuDat.m_EndTime;
          nsStringUtils::Copy(copiedGpuData.m_szName, GPUScope::NAME_SIZE, sourceGpuDat.m_szName);
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
void nsProfilingSystem::SetDiscardThreshold(nsTime threshold)
{
  cvar_ProfilingDiscardThresholdMS = static_cast<float>(threshold.GetMilliseconds());
}

void nsProfilingSystem::SetScopeTimeoutCallback(ScopeTimeoutDelegate callback)
{
  s_ScopeTimeoutCallback = callback;
}

// static
nsUInt64 nsProfilingSystem::GetFrameCount()
{
  return s_uiFrameCount;
}

// static
void nsProfilingSystem::StartNewFrame()
{
  ++s_uiFrameCount;

  if (!s_FrameStartTimes.CanAppend())
  {
    s_FrameStartTimes.PopFront();
  }

  s_FrameStartTimes.PushBack(nsTime::Now());

  NS_PROFILER_FRAME_MARKER();
}

// static
void nsProfilingSystem::AddCPUScope(nsStringView sName, const char* szFunctionName, nsTime beginTime, nsTime endTime, nsTime scopeTimeout)
{
  const nsTime duration = endTime - beginTime;

  // discard?
  if (duration < nsTime::MakeFromMilliseconds(cvar_ProfilingDiscardThresholdMS))
    return;

  ::CpuScopesBufferBase* pScopes = s_CpuScopes;

  if (pScopes == nullptr)
  {
    if (nsThreadUtils::IsMainThread())
    {
      pScopes = NS_DEFAULT_NEW(::CpuScopesBuffer<BUFFER_SIZE_MAIN_THREAD>);
    }
    else
    {
      pScopes = NS_DEFAULT_NEW(::CpuScopesBuffer<BUFFER_SIZE_OTHER_THREAD>);
    }

    pScopes->m_uiThreadId = (nsUInt64)nsThreadUtils::GetCurrentThreadID();
    s_CpuScopes = pScopes;

    {
      NS_LOCK(s_AllCpuScopesMutex);
      s_AllCpuScopes.PushBack(pScopes);
    }
  }

  CPUScope scope;
  scope.m_szFunctionName = szFunctionName;
  scope.m_BeginTime = beginTime;
  scope.m_EndTime = endTime;
  nsStringUtils::Copy(scope.m_szName, NS_ARRAY_SIZE(scope.m_szName), sName.GetStartPointer(), sName.GetEndPointer());

  if (nsThreadUtils::IsMainThread())
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
void nsProfilingSystem::Initialize()
{
  SetThreadName("Main Thread");

  s_MainThreadId = (nsUInt64)nsThreadUtils::GetCurrentThreadID();
}

// static
void nsProfilingSystem::Reset()
{
  NS_LOCK(s_ThreadInfosMutex);
  NS_LOCK(s_AllCpuScopesMutex);
  for (nsUInt32 i = 0; i < s_DeadThreadIDs.GetCount(); i++)
  {
    nsUInt64 uiThreadId = s_DeadThreadIDs[i];
    for (nsUInt32 k = 0; k < s_ThreadInfos.GetCount(); k++)
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
    for (nsUInt32 k = 0; k < s_AllCpuScopes.GetCount(); k++)
    {
      CpuScopesBufferBase* pEventBuffer = s_AllCpuScopes[k];
      if (pEventBuffer->m_uiThreadId == uiThreadId)
      {
        NS_DEFAULT_DELETE(pEventBuffer);
        // Forward order and no swap important, see comment above.
        s_AllCpuScopes.RemoveAtAndCopy(k);
      }
    }
  }
  s_DeadThreadIDs.Clear();
}

// static
void nsProfilingSystem::SetThreadName(nsStringView sThreadName)
{
  NS_LOCK(s_ThreadInfosMutex);

  ThreadInfo& info = s_ThreadInfos.ExpandAndGetRef();
  info.m_uiThreadId = (nsUInt64)nsThreadUtils::GetCurrentThreadID();
  info.m_sName = sThreadName;
}

// static
void nsProfilingSystem::RemoveThread()
{
  NS_LOCK(s_ThreadInfosMutex);

  s_DeadThreadIDs.PushBack((nsUInt64)nsThreadUtils::GetCurrentThreadID());
}

// static
void nsProfilingSystem::InitializeGPUData(nsUInt32 uiGpuCount)
{
  if (s_GPUScopes.GetCount() < uiGpuCount)
  {
    s_GPUScopes.SetCount(uiGpuCount);
  }

  for (auto& gpuScopes : s_GPUScopes)
  {
    if (gpuScopes == nullptr)
    {
      gpuScopes = NS_DEFAULT_NEW(GPUScopesBuffer);
    }
  }
}

void nsProfilingSystem::AddGPUScope(nsStringView sName, nsTime beginTime, nsTime endTime, nsUInt32 uiGpuIndex)
{
  // discard?
  if (endTime - beginTime < nsTime::MakeFromMilliseconds(cvar_ProfilingDiscardThresholdMS))
    return;

  if (!s_GPUScopes[uiGpuIndex]->CanAppend())
  {
    s_GPUScopes[uiGpuIndex]->PopFront();
  }

  GPUScope scope;
  scope.m_BeginTime = beginTime;
  scope.m_EndTime = endTime;
  nsStringUtils::Copy(scope.m_szName, NS_ARRAY_SIZE(scope.m_szName), sName.GetStartPointer(), sName.GetEndPointer());

  s_GPUScopes[uiGpuIndex]->PushBack(scope);
}

//////////////////////////////////////////////////////////////////////////

nsProfilingScope::nsProfilingScope(nsStringView sName, const char* szFunctionName, nsTime timeout)
  : m_sName(sName)
  , m_szFunction(szFunctionName)
  , m_BeginTime(nsTime::Now())
  , m_Timeout(timeout)
{
}

nsProfilingScope::~nsProfilingScope()
{
  nsProfilingSystem::AddCPUScope(m_sName, m_szFunction, m_BeginTime, nsTime::Now(), m_Timeout);
}

//////////////////////////////////////////////////////////////////////////

thread_local nsProfilingListScope* nsProfilingListScope::s_pCurrentList = nullptr;

nsProfilingListScope::nsProfilingListScope(nsStringView sListName, nsStringView sFirstSectionName, const char* szFunctionName)
  : m_sListName(sListName)
  , m_szListFunction(szFunctionName)
  , m_ListBeginTime(nsTime::Now())
  , m_sCurSectionName(sFirstSectionName)
  , m_CurSectionBeginTime(m_ListBeginTime)
{
  m_pPreviousList = s_pCurrentList;
  s_pCurrentList = this;
}

nsProfilingListScope::~nsProfilingListScope()
{
  nsTime now = nsTime::Now();
  nsProfilingSystem::AddCPUScope(m_sCurSectionName, nullptr, m_CurSectionBeginTime, now, nsTime::MakeZero());
  nsProfilingSystem::AddCPUScope(m_sListName, m_szListFunction, m_ListBeginTime, now, nsTime::MakeZero());

  s_pCurrentList = m_pPreviousList;
}

// static
void nsProfilingListScope::StartNextSection(nsStringView sNextSectionName)
{
  nsProfilingListScope* pCurScope = s_pCurrentList;

  nsTime now = nsTime::Now();
  nsProfilingSystem::AddCPUScope(pCurScope->m_sCurSectionName, nullptr, pCurScope->m_CurSectionBeginTime, now, nsTime::MakeZero());

  pCurScope->m_sCurSectionName = sNextSectionName;
  pCurScope->m_CurSectionBeginTime = now;
}

#else

nsResult nsProfilingSystem::ProfilingData::Write(nsStreamWriter& outputStream) const
{
  return NS_FAILURE;
}

void nsProfilingSystem::Clear() {}

void nsProfilingSystem::Capture(nsProfilingSystem::ProfilingData& out_Capture, bool bClearAfterCapture) {}

void nsProfilingSystem::SetDiscardThreshold(nsTime threshold) {}

void nsProfilingSystem::StartNewFrame() {}

void nsProfilingSystem::AddCPUScope(nsStringView sName, const char* szFunctionName, nsTime beginTime, nsTime endTime, nsTime scopeTimeout) {}

void nsProfilingSystem::Initialize() {}

void nsProfilingSystem::Reset() {}

void nsProfilingSystem::SetThreadName(nsStringView sThreadName) {}

void nsProfilingSystem::RemoveThread() {}

void nsProfilingSystem::InitializeGPUData(nsUInt32 gpuCount) {}

void nsProfilingSystem::AddGPUScope(nsStringView sName, nsTime beginTime, nsTime endTime, nsUInt32 gpuIndex) {}

void nsProfilingSystem::ProfilingData::Merge(ProfilingData& out_Merged, nsArrayPtr<const ProfilingData*> inputs) {}

#endif

NS_STATICLINK_FILE(Foundation, Foundation_Profiling_Implementation_Profiling);
