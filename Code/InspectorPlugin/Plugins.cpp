#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Configuration/Plugin.h>

namespace PluginsDetail
{
  static void SendPluginTelemetry()
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    nsTelemetry::Broadcast(nsTelemetry::Reliable, 'PLUG', ' CLR', nullptr, 0);

    nsHybridArray<nsPlugin::PluginInfo, 16> infos;
    nsPlugin::GetAllPluginInfos(infos);

    for (const auto& pi : infos)
    {
      nsTelemetryMessage msg;
      msg.SetMessageID('PLUG', 'DATA');
      msg.GetWriter() << pi.m_sName;
      msg.GetWriter() << false; // deprecated 'IsReloadable' flag

      nsStringBuilder s;

      for (const auto& dep : pi.m_sDependencies)
      {
        s.AppendWithSeparator(" | ", dep);
      }

      msg.GetWriter() << s;

      nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
    }
  }

  static void TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e)
  {
    switch (e.m_EventType)
    {
      case nsTelemetry::TelemetryEventData::ConnectedToClient:
        SendPluginTelemetry();
        break;

      default:
        break;
    }
  }

  static void PluginEventHandler(const nsPluginEvent& e)
  {
    switch (e.m_EventType)
    {
      case nsPluginEvent::AfterPluginChanges:
        SendPluginTelemetry();
        break;

      default:
        break;
    }
  }
} // namespace PluginsDetail

void AddPluginEventHandler()
{
  nsTelemetry::AddEventHandler(PluginsDetail::TelemetryEventsHandler);
  nsPlugin::Events().AddEventHandler(PluginsDetail::PluginEventHandler);
}

void RemovePluginEventHandler()
{
  nsPlugin::Events().RemoveEventHandler(PluginsDetail::PluginEventHandler);
  nsTelemetry::RemoveEventHandler(PluginsDetail::TelemetryEventsHandler);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_Plugins);
