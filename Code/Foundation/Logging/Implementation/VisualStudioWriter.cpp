#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/VisualStudioWriter.h>
#include <Foundation/Strings/StringConversion.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

void nsLogWriter::VisualStudio::LogMessageHandler(const nsLoggingEventData& eventData)
{
  if (eventData.m_EventType == nsLogMsgType::Flush)
    return;

  static nsMutex WriterLock; // will only be created if this writer is used at all
  NS_LOCK(WriterLock);

  if (eventData.m_EventType == nsLogMsgType::BeginGroup)
    OutputDebugStringA("\n");

  for (nsUInt32 i = 0; i < eventData.m_uiIndentation; ++i)
    OutputDebugStringA(" ");

  nsStringBuilder s;

  switch (eventData.m_EventType)
  {
    case nsLogMsgType::BeginGroup:
      s.SetFormat("+++++ {} ({}) +++++\n", eventData.m_sText, eventData.m_sTag);
      OutputDebugStringW(nsStringWChar(s));
      break;

    case nsLogMsgType::EndGroup:
#  if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
      s.SetFormat("----- {} ({} sec) -----\n\n", eventData.m_sText, eventData.m_fSeconds);
#  else
      s.SetFormat("----- {} (timing info not available) -----\n\n", eventData.m_sText);
#  endif
      OutputDebugStringW(nsStringWChar(s));
      break;

    case nsLogMsgType::ErrorMsg:
      s.SetFormat("Error: {}\n", eventData.m_sText);
      OutputDebugStringW(nsStringWChar(s));
      break;

    case nsLogMsgType::SeriousWarningMsg:
      s.SetFormat("Seriously: {}\n", eventData.m_sText);
      OutputDebugStringW(nsStringWChar(s));
      break;

    case nsLogMsgType::WarningMsg:
      s.SetFormat("Warning: {}\n", eventData.m_sText);
      OutputDebugStringW(nsStringWChar(s));
      break;

    case nsLogMsgType::SuccessMsg:
      s.SetFormat("{}\n", eventData.m_sText);
      OutputDebugStringW(nsStringWChar(s));
      break;

    case nsLogMsgType::InfoMsg:
      s.SetFormat("{}\n", eventData.m_sText);
      OutputDebugStringW(nsStringWChar(s));
      break;

    case nsLogMsgType::DevMsg:
      s.SetFormat("{}\n", eventData.m_sText);
      OutputDebugStringW(nsStringWChar(s));
      break;

    case nsLogMsgType::DebugMsg:
      s.SetFormat("{}\n", eventData.m_sText);
      OutputDebugStringW(nsStringWChar(s));
      break;

    default:
      s.SetFormat("{}\n", eventData.m_sText);
      OutputDebugStringW(nsStringWChar(s));

      nsLog::Warning("Unknown Message Type {0}", eventData.m_EventType);
      break;
  }
}

#else

void nsLogWriter::VisualStudio::LogMessageHandler(const nsLoggingEventData& eventData) {}

#endif
