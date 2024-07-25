#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/GlobalEvent.h>
#include <Foundation/Communication/Telemetry.h>

#include <Core/GameApplication/GameApplicationBase.h>

static nsGlobalEvent::EventMap s_LastState;

static void SendGlobalEventTelemetry(nsStringView sEvent, const nsGlobalEvent::EventData& ed)
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  nsTelemetryMessage msg;
  msg.SetMessageID('EVNT', 'DATA');
  msg.GetWriter() << sEvent;
  msg.GetWriter() << ed.m_uiNumTimesFired;
  msg.GetWriter() << ed.m_uiNumEventHandlersRegular;
  msg.GetWriter() << ed.m_uiNumEventHandlersOnce;

  nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
}

static void SendAllGlobalEventTelemetry()
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  // clear
  {
    nsTelemetryMessage msg;
    nsTelemetry::Broadcast(nsTelemetry::Reliable, 'EVNT', ' CLR', nullptr, 0);
  }

  nsGlobalEvent::UpdateGlobalEventStatistics();

  s_LastState = nsGlobalEvent::GetEventStatistics();

  for (nsGlobalEvent::EventMap::ConstIterator it = s_LastState.GetIterator(); it.IsValid(); ++it)
  {
    SendGlobalEventTelemetry(it.Key(), it.Value());
  }
}

static void SendChangedGlobalEventTelemetry()
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  static nsTime LastUpdate = nsTime::Now();

  if ((nsTime::Now() - LastUpdate).GetSeconds() < 0.5)
    return;

  LastUpdate = nsTime::Now();

  nsGlobalEvent::UpdateGlobalEventStatistics();

  const nsGlobalEvent::EventMap& data = nsGlobalEvent::GetEventStatistics();

  if (data.GetCount() != s_LastState.GetCount())
  {
    SendAllGlobalEventTelemetry();
    return;
  }

  for (nsGlobalEvent::EventMap::ConstIterator it = data.GetIterator(); it.IsValid(); ++it)
  {
    const nsGlobalEvent::EventData& currentEventData = it.Value();
    nsGlobalEvent::EventData& lastEventData = s_LastState[it.Key()];

    if (nsMemoryUtils::Compare(&currentEventData, &lastEventData) != 0)
    {
      SendGlobalEventTelemetry(it.Key().GetData(), it.Value());

      lastEventData = currentEventData;
    }
  }
}

namespace GlobalEventsDetail
{
  static void TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e)
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    switch (e.m_EventType)
    {
      case nsTelemetry::TelemetryEventData::ConnectedToClient:
        SendAllGlobalEventTelemetry();
        break;
      case nsTelemetry::TelemetryEventData::DisconnectedFromClient:
      {
        nsGlobalEvent::EventMap tmp;
        s_LastState.Swap(tmp);
        break;
      }
      default:
        break;
    }
  }

  static void PerframeUpdateHandler(const nsGameApplicationExecutionEvent& e)
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    switch (e.m_Type)
    {
      case nsGameApplicationExecutionEvent::Type::AfterPresent:
        SendChangedGlobalEventTelemetry();
        break;

      default:
        break;
    }
  }
} // namespace GlobalEventsDetail

void AddGlobalEventHandler()
{
  nsTelemetry::AddEventHandler(GlobalEventsDetail::TelemetryEventsHandler);

  // We're handling the per frame update by a different event since
  // using nsTelemetry::TelemetryEventData::PerFrameUpdate can lead
  // to deadlocks between the nsStats and nsTelemetry system.
  if (nsGameApplicationBase::GetGameApplicationBaseInstance() != nullptr)
  {
    nsGameApplicationBase::GetGameApplicationBaseInstance()->m_ExecutionEvents.AddEventHandler(GlobalEventsDetail::PerframeUpdateHandler);
  }
}

void RemoveGlobalEventHandler()
{
  if (nsGameApplicationBase::GetGameApplicationBaseInstance() != nullptr)
  {
    nsGameApplicationBase::GetGameApplicationBaseInstance()->m_ExecutionEvents.RemoveEventHandler(GlobalEventsDetail::PerframeUpdateHandler);
  }

  nsTelemetry::RemoveEventHandler(GlobalEventsDetail::TelemetryEventsHandler);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_GlobalEvents);
