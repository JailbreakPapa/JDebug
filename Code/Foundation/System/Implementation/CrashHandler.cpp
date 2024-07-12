#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/System/CrashHandler.h>
#include <Foundation/System/MiniDumpUtils.h>
#include <Foundation/System/Process.h>
#include <Foundation/Time/Timestamp.h>

//////////////////////////////////////////////////////////////////////////

nsCrashHandler* nsCrashHandler::s_pActiveHandler = nullptr;

nsCrashHandler::nsCrashHandler() = default;

nsCrashHandler::~nsCrashHandler()
{
  if (s_pActiveHandler == this)
  {
    SetCrashHandler(nullptr);
  }
}

nsCrashHandler* nsCrashHandler::GetCrashHandler()
{
  return s_pActiveHandler;
}

//////////////////////////////////////////////////////////////////////////

nsCrashHandler_WriteMiniDump nsCrashHandler_WriteMiniDump::g_Instance;

nsCrashHandler_WriteMiniDump::nsCrashHandler_WriteMiniDump() = default;

void nsCrashHandler_WriteMiniDump::SetFullDumpFilePath(nsStringView sFullAbsDumpFilePath)
{
  m_sDumpFilePath = sFullAbsDumpFilePath;
}

void nsCrashHandler_WriteMiniDump::SetDumpFilePath(nsStringView sAbsDirectoryPath, nsStringView sAppName, nsBitflags<PathFlags> flags)
{
  nsStringBuilder sOutputPath = sAbsDirectoryPath;

  if (flags.IsSet(PathFlags::AppendSubFolder))
  {
    sOutputPath.AppendPath("CrashDumps");
  }

  sOutputPath.AppendPath(sAppName);

  if (flags.IsSet(PathFlags::AppendDate))
  {
    const nsDateTime date = nsDateTime::MakeFromTimestamp(nsTimestamp::CurrentTimestamp());
    sOutputPath.AppendFormat("_{}", date);
  }

#if NS_ENABLED(NS_SUPPORTS_PROCESSES)
  if (flags.IsSet(PathFlags::AppendPID))
  {
    const nsUInt32 pid = nsProcess::GetCurrentProcessID();
    sOutputPath.AppendFormat("_{}", pid);
  }
#endif

  sOutputPath.Append(".dmp");

  SetFullDumpFilePath(sOutputPath);
}

void nsCrashHandler_WriteMiniDump::SetDumpFilePath(nsStringView sAppName, nsBitflags<PathFlags> flags)
{
  SetDumpFilePath(nsOSFile::GetApplicationDirectory(), sAppName, flags);
}

void nsCrashHandler_WriteMiniDump::HandleCrash(void* pOsSpecificData)
{
  bool crashDumpWritten = false;
  if (!m_sDumpFilePath.IsEmpty())
  {
#if NS_ENABLED(NS_SUPPORTS_CRASH_DUMPS)
    if (nsMiniDumpUtils::LaunchMiniDumpTool(m_sDumpFilePath).Failed())
    {
      nsLog::Print("Could not launch MiniDumpTool, trying to write crash-dump from crashed process directly.\n");

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
    nsLog::Print("nsCrashHandler_WriteMiniDump: No dump-file location specified.\n");
  }

  PrintStackTrace(pOsSpecificData);

  if (crashDumpWritten)
  {
    nsLog::Printf("Application crashed. Crash-dump written to '%s'\n.", m_sDumpFilePath.GetData());
  }
}
