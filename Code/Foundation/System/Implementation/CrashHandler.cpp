#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/System/CrashHandler.h>
#include <Foundation/System/MiniDumpUtils.h>
#include <Foundation/System/Process.h>
#include <Foundation/Time/Timestamp.h>

static void PrintHelper(const char* szString)
{
  wdLog::Printf("%s", szString);
}

//////////////////////////////////////////////////////////////////////////

wdCrashHandler* wdCrashHandler::s_pActiveHandler = nullptr;

wdCrashHandler::wdCrashHandler() = default;

wdCrashHandler::~wdCrashHandler()
{
  if (s_pActiveHandler == this)
  {
    SetCrashHandler(nullptr);
  }
}

wdCrashHandler* wdCrashHandler::GetCrashHandler()
{
  return s_pActiveHandler;
}

//////////////////////////////////////////////////////////////////////////

wdCrashHandler_WriteMiniDump wdCrashHandler_WriteMiniDump::g_Instance;

wdCrashHandler_WriteMiniDump::wdCrashHandler_WriteMiniDump() = default;

void wdCrashHandler_WriteMiniDump::SetFullDumpFilePath(wdStringView sFullAbsDumpFilePath)
{
  m_sDumpFilePath = sFullAbsDumpFilePath;
}

void wdCrashHandler_WriteMiniDump::SetDumpFilePath(wdStringView sAbsDirectoryPath, wdStringView sAppName, wdBitflags<PathFlags> flags)
{
  wdStringBuilder sOutputPath = sAbsDirectoryPath;

  if (flags.IsSet(PathFlags::AppendSubFolder))
  {
    sOutputPath.AppendPath("CrashDumps");
  }

  sOutputPath.AppendPath(sAppName);

  if (flags.IsSet(PathFlags::AppendDate))
  {
    const wdDateTime date = wdTimestamp::CurrentTimestamp();
    sOutputPath.AppendFormat("_{}", date);
  }

#if WD_ENABLED(WD_SUPPORTS_PROCESSES)
  if (flags.IsSet(PathFlags::AppendPID))
  {
    const wdUInt32 pid = wdProcess::GetCurrentProcessID();
    sOutputPath.AppendFormat("_{}", pid);
  }
#endif

  sOutputPath.Append(".dmp");

  SetFullDumpFilePath(sOutputPath);
}

void wdCrashHandler_WriteMiniDump::SetDumpFilePath(wdStringView sAppName, wdBitflags<PathFlags> flags)
{
  SetDumpFilePath(wdOSFile::GetApplicationDirectory(), sAppName, flags);
}

void wdCrashHandler_WriteMiniDump::HandleCrash(void* pOsSpecificData)
{
  bool crashDumpWritten = false;
  if (!m_sDumpFilePath.IsEmpty())
  {
#if WD_ENABLED(WD_SUPPORTS_CRASH_DUMPS)
    if (wdMiniDumpUtils::LaunchMiniDumpTool(m_sDumpFilePath).Failed())
    {
      wdLog::Print("Could not launch MiniDumpTool, trying to write crash-dump from crashed process directly.\n");

      crashDumpWritten = WriteOwnProcessMiniDump(pOsSpecificData);
    }
    else
    {
      crashDumpWritten = true;
    }
#else
    crashDumpWritten = WriteOwnProcessMiniDump(pOsSpecificData);
#endif
  }
  else
  {
    wdLog::Print("wdCrashHandler_WriteMiniDump: No dump-file location specified.\n");
  }

  PrintStackTrace(pOsSpecificData);

  if (crashDumpWritten)
  {
    wdLog::Printf("Application crashed. Crash-dump written to '%s'\n.", m_sDumpFilePath.GetData());
  }
}

//////////////////////////////////////////////////////////////////////////

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/System/Implementation/Win/CrashHandler_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/System/Implementation/Posix/CrashHandler_posix.h>
#else
#  error "wdCrashHandler is not implemented on current platform"
#endif


WD_STATICLINK_FILE(Foundation, Foundation_System_Implementation_CrashHandler);
