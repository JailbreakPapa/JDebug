#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/ConsoleWriter.h>
#include <Foundation/Time/Timestamp.h>

#if WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <android/log.h>
#  define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "wdEngine", __VA_ARGS__)
#endif

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

static void SetConsoleColor(WORD ui)
{
#  if WD_DISABLED(WD_PLATFORM_WINDOWS_UWP)
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ui);
#  endif
}
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
static void SetConsoleColor(wdUInt8 ui) {}
#else
#  error "Unknown Platform."
static void SetConsoleColor(wdUInt8 ui) {}
#endif

wdLog::TimestampMode wdLogWriter::Console::s_TimestampMode = wdLog::TimestampMode::None;

void wdLogWriter::Console::LogMessageHandler(const wdLoggingEventData& eventData)
{
  wdStringBuilder sTimestamp;
  wdLog::GenerateFormattedTimestamp(s_TimestampMode, sTimestamp);

  static wdMutex WriterLock; // will only be created if this writer is used at all
  WD_LOCK(WriterLock);

  if (eventData.m_EventType == wdLogMsgType::BeginGroup)
    printf("\n");

  for (wdUInt32 i = 0; i < eventData.m_uiIndentation; ++i)
    printf(" ");

  wdStringBuilder sTemp1, sTemp2;

  switch (eventData.m_EventType)
  {
    case wdLogMsgType::Flush:
      fflush(stdout);
      break;

    case wdLogMsgType::BeginGroup:
      SetConsoleColor(0x02);
      printf("+++++ %s (%s) +++++\n", eventData.m_sText.GetData(sTemp1), eventData.m_sTag.GetData(sTemp2));
      break;

    case wdLogMsgType::EndGroup:
      SetConsoleColor(0x02);
#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
      printf("----- %s (%.6f sec)-----\n\n", eventData.m_sText.GetData(sTemp1), eventData.m_fSeconds);
#else
      printf("----- %s (%s)-----\n\n", eventData.m_sText.GetData(sTemp1), "timing info not available");
#endif
      break;

    case wdLogMsgType::ErrorMsg:
      SetConsoleColor(0x0C);
      printf("%sError: %s\n", sTimestamp.GetData(), eventData.m_sText.GetData(sTemp1));
      fflush(stdout);
      break;

    case wdLogMsgType::SeriousWarningMsg:
      SetConsoleColor(0x0C);
      printf("%sSeriously: %s\n", sTimestamp.GetData(), eventData.m_sText.GetData(sTemp1));
      break;

    case wdLogMsgType::WarningMsg:
      SetConsoleColor(0x0E);
      printf("%sWarning: %s\n", sTimestamp.GetData(), eventData.m_sText.GetData(sTemp1));
      break;

    case wdLogMsgType::SuccessMsg:
      SetConsoleColor(0x0A);
      printf("%s%s\n", sTimestamp.GetData(), eventData.m_sText.GetData(sTemp1));
      fflush(stdout);
      break;

    case wdLogMsgType::InfoMsg:
      SetConsoleColor(0x07);
      printf("%s%s\n", sTimestamp.GetData(), eventData.m_sText.GetData(sTemp1));
      break;

    case wdLogMsgType::DevMsg:
      SetConsoleColor(0x08);
      printf("%s%s\n", sTimestamp.GetData(), eventData.m_sText.GetData(sTemp1));
      break;

    case wdLogMsgType::DebugMsg:
      SetConsoleColor(0x09);
      printf("%s%s\n", sTimestamp.GetData(), eventData.m_sText.GetData(sTemp1));
      break;

    default:
      SetConsoleColor(0x0D);
      printf("%s%s\n", sTimestamp.GetData(), eventData.m_sText.GetData(sTemp1));

      wdLog::Warning("Unknown Message Type {0}", eventData.m_EventType);
      break;
  }

  SetConsoleColor(0x07);
}

void wdLogWriter::Console::SetTimestampMode(wdLog::TimestampMode mode)
{
  s_TimestampMode = mode;
}
#if WD_ENABLED(WD_PLATFORM_ANDROID)
#  undef printf
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Logging_Implementation_ConsoleWriter);
