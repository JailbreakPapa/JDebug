#pragma once

#include <TestFramework/Framework/TestFramework.h>

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>

#if NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <android/log.h>
#endif
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Logging/ETWWriter.h>
inline void SetConsoleColorInl(WORD ui)
{
#  if NS_DISABLED(NS_PLATFORM_WINDOWS_UWP)
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), ui);
#  endif
}
#else
inline void SetConsoleColorInl(nsUInt8 ui) {}
#endif

inline void OutputToConsole(nsTestOutput::Enum type, const char* szMsg)
{
  static nsInt32 iIndentation = 0;
  static bool bAnyError = false;

  switch (type)
  {
    case nsTestOutput::StartOutput:
      break;
    case nsTestOutput::BeginBlock:
      iIndentation += 2;
      break;
    case nsTestOutput::EndBlock:
      iIndentation -= 2;
      break;
    case nsTestOutput::Details:
      SetConsoleColorInl(0x07);
      break;
    case nsTestOutput::ImportantInfo:
      SetConsoleColorInl(0x07);
      break;
    case nsTestOutput::Success:
      SetConsoleColorInl(0x0A);
      break;
    case nsTestOutput::Message:
      SetConsoleColorInl(0x0E);
      break;
    case nsTestOutput::Warning:
      SetConsoleColorInl(0x0C);
      break;
    case nsTestOutput::Error:
      SetConsoleColorInl(0x0C);
      bAnyError = true;
      break;
    case nsTestOutput::Duration:
    case nsTestOutput::ImageDiffFile:
    case nsTestOutput::InvalidType:
    case nsTestOutput::AllOutputTypes:
      return;

    case nsTestOutput::FinalResult:
      if (bAnyError)
        SetConsoleColorInl(0x0C);
      else
        SetConsoleColorInl(0x0A);

      // reset it for the next test round
      bAnyError = false;
      break;

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
  }

  printf("%*s%s\n", iIndentation, "", szMsg);
  SetConsoleColorInl(0x07);

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
  nsLogMsgType::Enum logType = nsLogMsgType::None;
  switch (Type)
  {
    case nsTestOutput::StartOutput:
    case nsTestOutput::InvalidType:
    case nsTestOutput::AllOutputTypes:
      logType = nsLogMsgType::None;
      break;
    case nsTestOutput::BeginBlock:
      logType = nsLogMsgType::BeginGroup;
      break;
    case nsTestOutput::EndBlock:
      logType = nsLogMsgType::EndGroup;
      break;
    case nsTestOutput::ImportantInfo:
    case nsTestOutput::Details:
    case nsTestOutput::Message:
    case nsTestOutput::Duration:
    case nsTestOutput::FinalResult:
      logType = nsLogMsgType::InfoMsg;
      break;
    case nsTestOutput::Success:
      logType = nsLogMsgType::SuccessMsg;
      break;
    case nsTestOutput::Warning:
      logType = nsLogMsgType::WarningMsg;
      break;
    case nsTestOutput::Error:
      logType = nsLogMsgType::ErrorMsg;
      break;
    case nsTestOutput::ImageDiffFile:
      logType = nsLogMsgType::DevMsg;
      break;
    default:
      break;
  }
  if (logType != nsLogMsgType::None)
  {
    nsLogWriter::ETW::LogMessage(nsLogMsgType::InfoMsg, iIndentation, szMsg);
  }
#endif
#if NS_ENABLED(NS_PLATFORM_WINDOWS)
  char sz[4096];
  nsStringUtils::snprintf(sz, 4096, "%*s%s\n", iIndentation, "", szMsg);
  OutputDebugStringW(nsStringWChar(sz).GetData());
#endif
#if NS_ENABLED(NS_PLATFORM_ANDROID)
  __android_log_print(ANDROID_LOG_DEBUG, "nsEngine", "%*s%s\n", iIndentation, "", szMsg);
#endif

  if (type >= nsTestOutput::Error)
  {
    fflush(stdout);
  }
}
