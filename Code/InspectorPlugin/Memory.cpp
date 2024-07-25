#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Memory/MemoryTracker.h>
#include <Foundation/Utilities/Stats.h>

#include <Core/GameApplication/GameApplicationBase.h>

namespace MemoryDetail
{

  static void BroadcastMemoryStats()
  {
    nsUInt64 uiTotalAllocations = 0;
    nsUInt64 uiTotalPerFrameAllocationSize = 0;
    nsTime TotalPerFrameAllocationTime;

    {
      nsTelemetryMessage msg;
      msg.SetMessageID(' MEM', 'BGN');
      nsTelemetry::Broadcast(nsTelemetry::Unreliable, msg);
    }

    for (auto it = nsMemoryTracker::GetIterator(); it.IsValid(); ++it)
    {
      nsTelemetryMessage msg;
      msg.SetMessageID(' MEM', 'STAT');
      msg.GetWriter() << it.Id().m_Data;
      msg.GetWriter() << it.Name();
      msg.GetWriter() << (it.ParentId().IsInvalidated() ? nsInvalidIndex : it.ParentId().m_Data);
      msg.GetWriter() << it.Stats();

      uiTotalAllocations += it.Stats().m_uiNumAllocations;
      uiTotalPerFrameAllocationSize += it.Stats().m_uiPerFrameAllocationSize;
      TotalPerFrameAllocationTime += it.Stats().m_PerFrameAllocationTime;

      nsTelemetry::Broadcast(nsTelemetry::Unreliable, msg);
    }

    {
      nsTelemetryMessage msg;
      msg.SetMessageID(' MEM', 'END');
      nsTelemetry::Broadcast(nsTelemetry::Unreliable, msg);
    }

    static nsUInt64 uiLastTotalAllocations = 0;

    nsStats::SetStat("App/Allocs Per Frame", uiTotalAllocations - uiLastTotalAllocations);
    nsStats::SetStat("App/Per Frame Alloc Size (byte)", uiTotalPerFrameAllocationSize);
    nsStats::SetStat("App/Per Frame Alloc Time", TotalPerFrameAllocationTime);

    uiLastTotalAllocations = uiTotalAllocations;

    nsMemoryTracker::ResetPerFrameAllocatorStats();
  }

  static void PerframeUpdateHandler(const nsGameApplicationExecutionEvent& e)
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    switch (e.m_Type)
    {
      case nsGameApplicationExecutionEvent::Type::AfterPresent:
        BroadcastMemoryStats();
        break;

      default:
        break;
    }
  }
} // namespace MemoryDetail


void AddMemoryEventHandler()
{
  // We're handling the per frame update by a different event since
  // using nsTelemetry::TelemetryEventData::PerFrameUpdate can lead
  // to deadlocks between the nsStats and nsTelemetry system.
  if (nsGameApplicationBase::GetGameApplicationBaseInstance() != nullptr)
  {
    nsGameApplicationBase::GetGameApplicationBaseInstance()->m_ExecutionEvents.AddEventHandler(MemoryDetail::PerframeUpdateHandler);
  }
}

void RemoveMemoryEventHandler()
{
  if (nsGameApplicationBase::GetGameApplicationBaseInstance() != nullptr)
  {
    nsGameApplicationBase::GetGameApplicationBaseInstance()->m_ExecutionEvents.RemoveEventHandler(MemoryDetail::PerframeUpdateHandler);
  }
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_Memory);
