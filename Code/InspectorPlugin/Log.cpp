#include <InspectorPlugin/InspectorPluginPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Logging/Log.h>

namespace nsLogWriter
{
  /// \brief This log-writer will broadcast all messages through nsTelemetry, such that external applications can display the log messages.
  class Telemetry
  {
  public:
    /// \brief Register this at nsLog to broadcast all log messages through nsTelemetry.
    static void LogMessageHandler(const nsLoggingEventData& eventData)
    {
      nsTelemetryMessage msg;
      msg.SetMessageID(' LOG', ' MSG');

      msg.GetWriter() << (nsInt8)eventData.m_EventType;
      msg.GetWriter() << (nsUInt8)eventData.m_uiIndentation;
      msg.GetWriter() << eventData.m_sTag;
      msg.GetWriter() << eventData.m_sText;

      if (eventData.m_EventType == nsLogMsgType::EndGroup)
      {
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
        msg.GetWriter() << eventData.m_fSeconds;
#else
        msg.GetWriter() << 0.0f;
#endif
      }

      nsTelemetry::Broadcast(nsTelemetry::Reliable, msg);
    }
  };
} // namespace nsLogWriter

void AddLogWriter()
{
  nsGlobalLog::AddLogWriter(&nsLogWriter::Telemetry::LogMessageHandler);
}

void RemoveLogWriter()
{
  nsGlobalLog::RemoveLogWriter(&nsLogWriter::Telemetry::LogMessageHandler);
}



NS_STATICLINK_FILE(InspectorPlugin, InspectorPlugin_Log);
