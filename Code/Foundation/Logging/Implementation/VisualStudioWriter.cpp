#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Strings/StringConversion.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

void wdLogWriter::VisualStudio::LogMessageHandler(const wdLoggingEventData& eventData)
{
  if (eventData.m_EventType == wdLogMsgType::Flush)
    return;

  static wdMutex WriterLock; // will only be created if this writer is used at all
  WD_LOCK(WriterLock);

  if (eventData.m_EventType == wdLogMsgType::BeginGroup)
    OutputDebugStringA("\n");

  for (wdUInt32 i = 0; i < eventData.m_uiIndentation; ++i)
    OutputDebugStringA(" ");

  wdStringBuilder s;

  switch (eventData.m_EventType)
  {
    case wdLogMsgType::BeginGroup:
      s.Format("+++++ {} ({}) +++++\n", eventData.m_sText, eventData.m_sTag);
      OutputDebugStringW(wdStringWChar(s));
      break;

    case wdLogMsgType::EndGroup:
#  if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
      s.Format("----- {} ({} sec) -----\n\n", eventData.m_sText, eventData.m_fSeconds);
#  else
      s.Format("----- {} (timing info not available) -----\n\n", eventData.m_sText);
#  endif
      OutputDebugStringW(wdStringWChar(s));
      break;

    case wdLogMsgType::ErrorMsg:
      s.Format("Error: {}\n", eventData.m_sText);
      OutputDebugStringW(wdStringWChar(s));
      break;

    case wdLogMsgType::SeriousWarningMsg:
      s.Format("Seriously: {}\n", eventData.m_sText);
      OutputDebugStringW(wdStringWChar(s));
      break;

    case wdLogMsgType::WarningMsg:
      s.Format("Warning: {}\n", eventData.m_sText);
      OutputDebugStringW(wdStringWChar(s));
      break;

    case wdLogMsgType::SuccessMsg:
      s.Format("{}\n", eventData.m_sText);
      OutputDebugStringW(wdStringWChar(s));
      break;

    case wdLogMsgType::InfoMsg:
      s.Format("{}\n", eventData.m_sText);
      OutputDebugStringW(wdStringWChar(s));
      break;

    case wdLogMsgType::DevMsg:
      s.Format("{}\n", eventData.m_sText);
      OutputDebugStringW(wdStringWChar(s));
      break;

    case wdLogMsgType::DebugMsg:
      s.Format("{}\n", eventData.m_sText);
      OutputDebugStringW(wdStringWChar(s));
      break;

    default:
      s.Format("{}\n", eventData.m_sText);
      OutputDebugStringW(wdStringWChar(s));

      wdLog::Warning("Unknown Message Type {0}", eventData.m_EventType);
      break;
  }
}

#else

void wdLogWriter::VisualStudio::LogMessageHandler(const wdLoggingEventData& eventData) {}

#endif



WD_STATICLINK_FILE(Foundation, Foundation_Logging_Implementation_VisualStudioWriter);
