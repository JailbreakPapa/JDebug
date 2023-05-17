#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/ETWWriter.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Logging/Implementation/Win/ETWProvider_win.h>

void wdLogWriter::ETW::LogMessageHandler(const wdLoggingEventData& eventData)
{
  if (eventData.m_EventType == wdLogMsgType::Flush)
    return;

  wdETWProvider::GetInstance().LogMessge(eventData.m_EventType, eventData.m_uiIndentation, eventData.m_sText);
}

void wdLogWriter::ETW::LogMessage(wdLogMsgType::Enum eventType, wdUInt8 uiIndentation, wdStringView sText)
{
  if (eventType == wdLogMsgType::Flush)
    return;

  wdETWProvider::GetInstance().LogMessge(eventType, uiIndentation, sText);
}

#else

void wdLogWriter::ETW::LogMessageHandler(const wdLoggingEventData& eventData) {}

void wdLogWriter::ETW::LogMessage(wdLogMsgType::Enum eventType, wdUInt8 uiIndentation, wdStringView sText) {}

#endif

WD_STATICLINK_FILE(Foundation, Foundation_Logging_Implementation_ETWWriter);
