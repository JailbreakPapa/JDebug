#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/ETWWriter.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS) || (NS_ENABLED(NS_PLATFORM_LINUX) && defined(BUILDSYSTEM_ENABLE_TRACELOGGING_LTTNG_SUPPORT))

#  if NS_ENABLED(NS_PLATFORM_WINDOWS)
#    include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#    include <Foundation/Platform/Win/ETWProvider_Win.h>
#  else
#    include <Foundation/Platform/Linux/ETWProvider_Linux.h>
#  endif

void nsLogWriter::ETW::LogMessageHandler(const nsLoggingEventData& eventData)
{
  if (eventData.m_EventType == nsLogMsgType::Flush)
    return;

  nsETWProvider::GetInstance().LogMessage(eventData.m_EventType, eventData.m_uiIndentation, eventData.m_sText);
}

void nsLogWriter::ETW::LogMessage(nsLogMsgType::Enum eventType, nsUInt8 uiIndentation, nsStringView sText)
{
  if (eventType == nsLogMsgType::Flush)
    return;

  nsETWProvider::GetInstance().LogMessage(eventType, uiIndentation, sText);
}

#else

void nsLogWriter::ETW::LogMessageHandler(const nsLoggingEventData& eventData) {}

void nsLogWriter::ETW::LogMessage(nsLogMsgType::Enum eventType, nsUInt8 uiIndentation, nsStringView sText) {}

#endif
