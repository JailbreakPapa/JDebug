#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/GlobalEvent.h>
#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Configuration/Startup.h>

namespace StartupDetail
{
  static void SendSubsystemTelemetry();
  static nsInt32 s_iSendSubSystemTelemetry = 0;
} // namespace StartupDetail

NS_ON_GLOBAL_EVENT(nsStartup_StartupCoreSystems_End)
{
  StartupDetail::SendSubsystemTelemetry();
}

NS_ON_GLOBAL_EVENT(nsStartup_StartupHighLevelSystems_End)
{
  StartupDetail::SendSubsystemTelemetry();
}

NS_ON_GLOBAL_EVENT(nsStartup_ShutdownCoreSystems_End)
{
  StartupDetail::SendSubsystemTelemetry();
}

NS_ON_GLOBAL_EVENT(nsStartup_ShutdownHighLevelSystems_End)
{
  StartupDetail::SendSubsystemTelemetry();
}

namespace StartupDetail
{
  static void SendSubsystemTelemetry()
  {
    if (s_iSendSubSystemTelemetry <= 0)
      return;

    nsTelemetry::Broadcast(nsTelemetry::Reliable, 'STRT', ' CLR', nullptr, 0);

    nsSubSystem* pSub = nsSubSystem::GetFirstInstance();

    while (pSub)
    {
      nsTelemetryMessage msg;
      msg.SetMessageID('STRT', 'SYST');
      msg.GetWriter() << pSub->GetGroupName();
      msg.GetWriter() << pSub->GetSubSystemName();
      msg.GetWriter() << pSub->GetPluginName();

      for (nsUInt32 i = 0; i < nsStartupStage::ENUM_COUNT; ++i)
        msg.GetWriter() << pSub->IsStartupPhaseDone((nsStartupStage::Enum)i);

      nsUInt8 uiDependencies = 0;
      while (pSub->GetDependency(uiDependencies) != nullptr)
        ++uiDependencies;

      msg.GetWriter() << uiDependencies;

      for (nsUInt8 i = 0; i < uiDependencies; ++i)
        msg.GetWriter() << pSub->GetDependency(i);

      nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);

      pSub = pSub->GetNextInstance();
    }
  }

  static void TelemetryEventsHandler(const nsTelemetry::TelemetryEventData& e)
  {
    if (!nsTelemetry::IsConnectedToClient())
      return;

    switch (e.m_EventType)
    {
      case nsTelemetry::TelemetryEventData::ConnectedToClient:
        SendSubsystemTelemetry();
        break;

      default:
        break;
    }
  }
} // namespace StartupDetail

void AddStartupEventHandler()
{
  ++StartupDetail::s_iSendSubSystemTelemetry;
  nsTelemetry::AddEventHandler(StartupDetail::TelemetryEventsHandler);
}

void RemoveStartupEventHandler()
{
  --StartupDetail::s_iSendSubSystemTelemetry;
  nsTelemetry::RemoveEventHandler(StartupDetail::TelemetryEventsHandler);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_Startup);
