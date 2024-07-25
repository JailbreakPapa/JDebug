#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Foundation/Communication/Telemetry.h>

namespace ResourceManagerDetail
{

  static void SendFullResourceInfo(const nsResource* pRes)
  {
    nsTelemetryMessage Msg;

    Msg.SetMessageID('RESM', ' SET');

    Msg.GetWriter() << pRes->GetResourceIDHash();
    Msg.GetWriter() << pRes->GetResourceID();
    Msg.GetWriter() << pRes->GetDynamicRTTI()->GetTypeName();
    Msg.GetWriter() << static_cast<nsUInt8>(pRes->GetPriority());
    Msg.GetWriter() << static_cast<nsUInt8>(pRes->GetBaseResourceFlags().GetValue());
    Msg.GetWriter() << static_cast<nsUInt8>(pRes->GetLoadingState());
    Msg.GetWriter() << pRes->GetNumQualityLevelsDiscardable();
    Msg.GetWriter() << pRes->GetNumQualityLevelsLoadable();
    Msg.GetWriter() << pRes->GetMemoryUsage().m_uiMemoryCPU;
    Msg.GetWriter() << pRes->GetMemoryUsage().m_uiMemoryGPU;
    Msg.GetWriter() << pRes->GetResourceDescription();

    nsTelemetry::Broadcast(nsTelemetry::Reliable, Msg);
  }

  static void SendSmallResourceInfo(const nsResource* pRes)
  {
    nsTelemetryMessage Msg;

    Msg.SetMessageID('RESM', 'UPDT');

    Msg.GetWriter() << pRes->GetResourceIDHash();
    Msg.GetWriter() << static_cast<nsUInt8>(pRes->GetPriority());
    Msg.GetWriter() << static_cast<nsUInt8>(pRes->GetBaseResourceFlags().GetValue());
    Msg.GetWriter() << static_cast<nsUInt8>(pRes->GetLoadingState());
    Msg.GetWriter() << pRes->GetNumQualityLevelsDiscardable();
    Msg.GetWriter() << pRes->GetNumQualityLevelsLoadable();
    Msg.GetWriter() << pRes->GetMemoryUsage().m_uiMemoryCPU;
    Msg.GetWriter() << pRes->GetMemoryUsage().m_uiMemoryGPU;

    nsTelemetry::Broadcast(nsTelemetry::Reliable, Msg);
  }

  static void SendDeleteResourceInfo(const nsResource* pRes)
  {
    nsTelemetryMessage Msg;

    Msg.SetMessageID('RESM', ' DEL');

    Msg.GetWriter() << pRes->GetResourceIDHash();

    nsTelemetry::Broadcast(nsTelemetry::Reliable, Msg);
  }

  static void SendAllResourceTelemetry()
  {
    nsResourceManager::BroadcastExistsEvent();
  }

  static void TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e)
  {
    switch (e.m_EventType)
    {
      case nsTelemetry::TelemetryEventData::ConnectedToClient:
        SendAllResourceTelemetry();
        break;

      default:
        break;
    }
  }

  static void ResourceManagerEventHandler(const nsResourceEvent& e)
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    switch (e.m_Type)
    {
      case nsResourceEvent::Type::ResourceCreated:
      case nsResourceEvent::Type::ResourceExists:
        SendFullResourceInfo(e.m_pResource);
        return;

      case nsResourceEvent::Type::ResourceDeleted:
        SendDeleteResourceInfo(e.m_pResource);
        return;

      case nsResourceEvent::Type::ResourceContentUpdated:
      case nsResourceEvent::Type::ResourceContentUnloading:
      case nsResourceEvent::Type::ResourcePriorityChanged:
        SendSmallResourceInfo(e.m_pResource);
        return;

      default:
        NS_ASSERT_NOT_IMPLEMENTED;
    }
  }
} // namespace ResourceManagerDetail

void AddResourceManagerEventHandler()
{
  nsTelemetry::AddEventHandler(ResourceManagerDetail::TelemetryEventsHandler);
  nsResourceManager::GetResourceEvents().AddEventHandler(ResourceManagerDetail::ResourceManagerEventHandler);
}

void RemoveResourceManagerEventHandler()
{
  nsResourceManager::GetResourceEvents().RemoveEventHandler(ResourceManagerDetail::ResourceManagerEventHandler);
  nsTelemetry::RemoveEventHandler(ResourceManagerDetail::TelemetryEventsHandler);
}
