#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Time/Clock.h>

static void TimeEventHandler(const nsClock::EventData& e)
{
  if (!nsTelemetry::IsConnectedToClient())
    return;

  nsTelemetryMessage Msg;
  Msg.SetMessageID('TIME', 'UPDT');
  Msg.GetWriter() << e.m_sClockName;
  Msg.GetWriter() << nsTime::Now();
  Msg.GetWriter() << e.m_RawTimeStep;
  Msg.GetWriter() << e.m_SmoothedTimeStep;

  nsTelemetry::Broadcast(nsTelemetry::Unreliable, Msg);
}

void AddTimeEventHandler()
{
  nsClock::AddEventHandler(TimeEventHandler);
}

void RemoveTimeEventHandler()
{
  nsClock::RemoveEventHandler(TimeEventHandler);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_Time);
