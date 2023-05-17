#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/Win/MinWindows.h>
#include <Foundation/IO/Implementation/Win/DosDevicePath_win.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/System/MiniDumpUtils.h>
#include <Foundation/System/ProcessGroup.h>
#include <Foundation/Types/ScopeExit.h>
#include <Foundation/Utilities/CommandLineOptions.h>
#include <Foundation/Utilities/CommandLineUtils.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)

#  include <Dbghelp.h>
#  include <Shlwapi.h>
#  include <tchar.h>
#  include <werapi.h>

wdCommandLineOptionBool opt_FullCrashDumps("app", "-fullcrashdumps", "If enabled, crash dumps will contain the full memory image.", false);

typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
  PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

wdMinWindows::HANDLE wdMiniDumpUtils::GetProcessHandleWithNecessaryRights(wdUInt32 uiProcessID)
{
  // try to get more than we need
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, uiProcessID);

  if (hProcess == NULL)
  {
    // try to get all that we need for a nice dump
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_DUP_HANDLE, FALSE, uiProcessID);
  }

  if (hProcess == NULL)
  {
    // try to get rights for a limited dump
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, uiProcessID);
  }

  return hProcess;
}

wdStatus wdMiniDumpUtils::WriteProcessMiniDump(
  const char* szDumpFile, wdUInt32 uiProcessID, wdMinWindows::HANDLE pProcess, struct _EXCEPTION_POINTERS* pExceptionInfo)
{
  HMODULE hDLL = ::LoadLibraryA("dbghelp.dll");

  if (hDLL == nullptr)
  {
    return wdStatus("dbghelp.dll could not be loaded.");
  }

  MINIDUMPWRITEDUMP MiniDumpWriteDumpFunc = (MINIDUMPWRITEDUMP)::GetProcAddress(hDLL, "MiniDumpWriteDump");

  if (MiniDumpWriteDumpFunc == nullptr)
  {
    return wdStatus("'MiniDumpWriteDump' function address could not be resolved.");
  }

  wdUInt32 dumpType = MiniDumpWithHandleData | MiniDumpWithModuleHeaders | MiniDumpWithUnloadedModules | MiniDumpWithProcessThreadData |
                      MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo;

  if (opt_FullCrashDumps.GetOptionValue(wdCommandLineOption::LogMode::Always))
  {
    dumpType |= MiniDumpWithFullMemory;
  }

  // make sure the target folder exists
  {
    wdStringBuilder folder = szDumpFile;
    folder.PathParentDirectory();
    if (wdOSFile::CreateDirectoryStructure(folder).Failed())
      return wdStatus("Failed to create output directory structure.");
  }

  HANDLE hFile = CreateFileW(wdDosDevicePath(szDumpFile), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hFile == INVALID_HANDLE_VALUE)
  {
    return wdStatus(wdFmt("Creating dump file '{}' failed (Error: '{}').", szDumpFile, wdArgErrorCode(GetLastError())));
  }

  WD_SCOPE_EXIT(CloseHandle(hFile););

  MINIDUMP_EXCEPTION_INFORMATION exceptionParam;
  exceptionParam.ThreadId = GetCurrentThreadId(); // only valid for WriteOwnProcessMiniDump()
  exceptionParam.ExceptionPointers = pExceptionInfo;
  exceptionParam.ClientPointers = TRUE;

  if (MiniDumpWriteDumpFunc(
        pProcess, uiProcessID, hFile, (MINIDUMP_TYPE)dumpType, pExceptionInfo != nullptr ? &exceptionParam : nullptr, nullptr, nullptr) == FALSE)
  {
    return wdStatus(wdFmt("Writing dump file failed: '{}'.", wdArgErrorCode(GetLastError())));
  }

  return wdStatus(WD_SUCCESS);
}

wdStatus wdMiniDumpUtils::WriteOwnProcessMiniDump(const char* szDumpFile, struct _EXCEPTION_POINTERS* pExceptionInfo)
{
  return WriteProcessMiniDump(szDumpFile, GetCurrentProcessId(), GetCurrentProcess(), pExceptionInfo);
}

wdStatus wdMiniDumpUtils::WriteExternalProcessMiniDump(const char* szDumpFile, wdUInt32 uiProcessID, wdMinWindows::HANDLE pProcess)
{
  return WriteProcessMiniDump(szDumpFile, uiProcessID, pProcess, nullptr);
}

#endif

wdStatus wdMiniDumpUtils::WriteExternalProcessMiniDump(const char* szDumpFile, wdUInt32 uiProcessID)
{
#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
  HANDLE hProcess = wdMiniDumpUtils::GetProcessHandleWithNecessaryRights(uiProcessID);

  if (hProcess == nullptr)
  {
    return wdStatus("Cannot access process for mini-dump writing (PID invalid or not enough rights).");
  }

  return WriteProcessMiniDump(szDumpFile, uiProcessID, hProcess, nullptr);

#else
  return wdStatus("Not implemented on UPW");
#endif
}

wdStatus wdMiniDumpUtils::LaunchMiniDumpTool(const char* szDumpFile)
{
#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
  wdStringBuilder sDumpToolPath = wdOSFile::GetApplicationDirectory();
  sDumpToolPath.AppendPath("MiniDumpTool.exe");
  sDumpToolPath.MakeCleanPath();

  if (!wdOSFile::ExistsFile(sDumpToolPath))
    return wdStatus(wdFmt("MiniDumpTool.exe not found in '{}'", sDumpToolPath));

  wdProcessOptions procOpt;
  procOpt.m_sProcess = sDumpToolPath;
  procOpt.m_Arguments.PushBack("-PID");
  procOpt.AddArgument("{}", wdProcess::GetCurrentProcessID());
  procOpt.m_Arguments.PushBack("-f");
  procOpt.m_Arguments.PushBack(szDumpFile);

  if (opt_FullCrashDumps.GetOptionValue(wdCommandLineOption::LogMode::Always))
  {
    // forward the '-fullcrashdumps' command line argument
    procOpt.AddArgument("-fullcrashdumps");
  }

  wdProcessGroup proc;
  if (proc.Launch(procOpt).Failed())
    return wdStatus(wdFmt("Failed to launch '{}'", sDumpToolPath));

  if (proc.WaitToFinish().Failed())
    return wdStatus("Waiting for MiniDumpTool to finish failed.");

  return wdStatus(WD_SUCCESS);

#else
  return wdStatus("Not implemented on UPW");
#endif
}
