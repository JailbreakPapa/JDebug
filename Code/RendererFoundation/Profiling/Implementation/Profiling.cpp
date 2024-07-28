#include <RendererFoundation/RendererFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Profiling/Profiling.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Profiling/Profiling.h>

#if NS_ENABLED(NS_USE_PROFILING)

struct GPUTimingScope
{
  NS_DECLARE_POD_TYPE();

  nsGALTimestampHandle m_BeginTimestamp;
  nsGALTimestampHandle m_EndTimestamp;
  char m_szName[48];
};

class GPUProfilingSystem
{
public:
  static void ProcessTimestamps(const nsGALDeviceEvent& e)
  {
    if (e.m_Type != nsGALDeviceEvent::AfterEndFrame)
      return;

    while (!s_TimingScopes.IsEmpty())
    {
      auto& timingScope = s_TimingScopes.PeekFront();

      nsTime endTime;
      if (e.m_pDevice->GetTimestampResult(timingScope.m_EndTimestamp, endTime).Succeeded())
      {
        nsTime beginTime;
        NS_VERIFY(e.m_pDevice->GetTimestampResult(timingScope.m_BeginTimestamp, beginTime).Succeeded(),
          "Begin timestamp should be finished before end timestamp");

        if (!beginTime.IsZero() && !endTime.IsZero())
        {
#  if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
          static bool warnOnRingBufferOverun = true;
          if (warnOnRingBufferOverun && endTime < beginTime)
          {
            warnOnRingBufferOverun = false;
            nsLog::Warning("Profiling end is before start, the DX11 timestamp ring buffer was probably overrun.");
          }
#  endif
          nsProfilingSystem::AddGPUScope(timingScope.m_szName, beginTime, endTime);
        }

        s_TimingScopes.PopFront();
      }
      else
      {
        // Timestamps are not available yet
        break;
      }
    }
  }

  static GPUTimingScope& AllocateScope() { return s_TimingScopes.ExpandAndGetRef(); }

private:
  static void OnEngineStartup() { nsGALDevice::GetDefaultDevice()->s_Events.AddEventHandler(&GPUProfilingSystem::ProcessTimestamps); }

  static void OnEngineShutdown()
  {
    s_TimingScopes.Clear();
    nsGALDevice::GetDefaultDevice()->s_Events.RemoveEventHandler(&GPUProfilingSystem::ProcessTimestamps);
  }

  static nsDeque<GPUTimingScope, nsStaticsAllocatorWrapper> s_TimingScopes;

  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererFoundation, GPUProfilingSystem);
};

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererFoundation, GPUProfilingSystem)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    GPUProfilingSystem::OnEngineStartup();
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    GPUProfilingSystem::OnEngineShutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsDeque<GPUTimingScope, nsStaticsAllocatorWrapper> GPUProfilingSystem::s_TimingScopes;

//////////////////////////////////////////////////////////////////////////

GPUTimingScope* nsProfilingScopeAndMarker::Start(nsGALCommandEncoder* pCommandEncoder, const char* szName)
{
  pCommandEncoder->PushMarker(szName);

  auto& timingScope = GPUProfilingSystem::AllocateScope();
  timingScope.m_BeginTimestamp = pCommandEncoder->InsertTimestamp();
  nsStringUtils::Copy(timingScope.m_szName, NS_ARRAY_SIZE(timingScope.m_szName), szName);

  return &timingScope;
}

void nsProfilingScopeAndMarker::Stop(nsGALCommandEncoder* pCommandEncoder, GPUTimingScope*& ref_pTimingScope)
{
  pCommandEncoder->PopMarker();
  ref_pTimingScope->m_EndTimestamp = pCommandEncoder->InsertTimestamp();
  ref_pTimingScope = nullptr;
}

nsProfilingScopeAndMarker::nsProfilingScopeAndMarker(nsGALCommandEncoder* pCommandEncoder, const char* szName)
  : nsProfilingScope(szName, nullptr, nsTime::MakeZero())
  , m_pCommandEncoder(pCommandEncoder)
{
  m_pTimingScope = Start(pCommandEncoder, szName);
}

nsProfilingScopeAndMarker::~nsProfilingScopeAndMarker()
{
  Stop(m_pCommandEncoder, m_pTimingScope);
}

#endif

NS_STATICLINK_FILE(RendererFoundation, RendererFoundation_Profiling_Implementation_Profiling);
