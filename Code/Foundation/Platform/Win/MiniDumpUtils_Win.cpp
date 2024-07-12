#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Basics/Platform/Win/MinWindows.h>
#  include <Foundation/IO/OSFile.h>
#  include <Foundation/Platform/Win/DosDevicePath_Win.h>
#  include <Foundation/System/MiniDumpUtils.h>
#  include <Foundation/System/ProcessGroup.h>
#  include <Foundation/Types/ScopeExit.h>
#  include <Foundation/Utilities/CommandLineOptions.h>
#  include <Foundation/Utilities/CommandLineUtils.h>

#  if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)

#    include <Dbghelp.h>
#    include <Shlwapi.h>
#    include <tchar.h>
#    include <werapi.h>

nsCommandLineOptionBool opt_FullCrashDumps("app", "-fullcrashdumps", "If enabled, crash dumps will contain the full memory image.", false);

using MINIDUMPWRITEDUMP = BOOL(WINAPI*)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType,
  PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

nsMinWindows::HANDLE nsMiniDumpUtils::GetProcessHandleWithNecessaryRights(nsUInt32 uiProcessID)
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

nsStatus nsMiniDumpUtils::WriteProcessMiniDump(nsStringView sDumpFile, nsUInt32 uiProcessID, nsMinWindows::HANDLE hProcess, struct _EXCEPTION_POINTERS* pExceptionInfo, nsDumpType dumpTypeOverride)
{
  HMODULE hDLL = ::LoadLibraryA("dbghelp.dll");

  if (hDLL == nullptr)
  {
    return nsStatus("dbghelp.dll could not be loaded.");
  }

  MINIDUMPWRITEDUMP MiniDumpWriteDumpFunc = (MINIDUMPWRITEDUMP)::GetProcAddress(hDLL, "MiniDumpWriteDump");

  if (MiniDumpWriteDumpFunc == nullptr)
  {
    return nsStatus("'MiniDumpWriteDump' function address could not be resolved.");
  }

  nsUInt32 dumpType = MiniDumpWithHandleData | MiniDumpWithModuleHeaders | MiniDumpWithUnloadedModules | MiniDumpWithProcessThreadData |
                      MiniDumpWithFullMemoryInfo | MiniDumpWithThreadInfo;

  if ((opt_FullCrashDumps.GetOptionValue(nsCommandLineOption::LogMode::Always) && dumpTypeOverride == nsDumpType::Auto) || dumpTypeOverride == nsDumpType::MiniDumpWithFullMemory)
  {
    dumpType |= MiniDumpWithFullMemory;
  }

  // make sure the target folder exists
  {
    nsStringBuilder folder = sDumpFile;
    folder.PathParentDirectory();
    if (nsOSFile::CreateDirectoryStructure(folder).Failed())
      return nsStatus("Failed to create output directory structure.");
  }

  HANDLE hFile = CreateFileW(nsDosDevicePath(sDumpFile), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

  if (hFile == INVALID_HANDLE_VALUE)
  {
    return nsStatus(nsFmt("Creating dump file '{}' failed (Error: '{}').", sDumpFile, nsArgErrorCode(GetLastError())));
  }

  NS_SCOPE_EXIT(CloseHandle(hFile););

  MINIDUMP_EXCEPTION_INFORMATION exceptionParam;
  exceptionParam.ThreadId = GetCurrentThreadId(); // only valid for WriteOwnProcessMiniDump()
  exceptionParam.ExceptionPointers = pExceptionInfo;
  exceptionParam.ClientPointers = TRUE;

  if (MiniDumpWriteDumpFunc(
        hProcess, uiProcessID, hFile, (MINIDUMP_TYPE)dumpType, pExceptionInfo != nullptr ? &exceptionParam : nullptr, nullptr, nullptr) == FALSE)
  {
    return nsStatus(nsFmt("Writing dump file failed: '{}'.", nsArgErrorCode(GetLastError())));
  }

  return nsStatus(NS_SUCCESS);
}

nsStatus nsMiniDumpUtils::WriteOwnProcessMiniDump(nsStringView sDumpFile, struct _EXCEPTION_POINTERS* pExceptionInfo, nsDumpType dumpTypeOverride)
{
  return WriteProcessMiniDump(sDumpFile, GetCurrentProcessId(), GetCurrentProcess(), pExceptionInfo, dumpTypeOverride);
}

nsStatus nsMiniDumpUtils::WriteExternalProcessMiniDump(nsStringView sDumpFile, nsUInt32 uiProcessID, nsMinWindows::HANDLE hProcess, nsDumpType dumpTypeOverride)
{
  return WriteProcessMiniDump(sDumpFile, uiProcessID, hProcess, nullptr, dumpTypeOverride);
}

#  endif

nsStatus nsMiniDumpUtils::WriteExternalProcessMiniDump(nsStringView sDumpFile, nsUInt32 uiProcessID, nsDumpType dumpTypeOverride)
{
#  if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
  HANDLE hProcess = nsMiniDumpUtils::GetProcessHandleWithNecessaryRights(uiProcessID);

  if (hProcess == nullptr)
  {
    return nsStatus("Cannot access process for mini-dump writing (PID invalid or not enough rights).");
  }

  return WriteProcessMiniDump(sDumpFile, uiProcessID, hProcess, nullptr, dumpTypeOverride);

#  else
  return nsStatus("Not implemented on UPW");
#  endif
}

nsStatus nsMiniDumpUtils::LaunchMiniDumpTool(nsStringView sDumpFile, nsDumpType dumpTypeOverride)
{
#  if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
  nsStringBuilder sDumpToolPath = nsOSFile::GetApplicationDirectory();
  sDumpToolPath.AppendPath("MiniDumpTool.exe");
  sDumpToolPath.MakeCleanPath();

  if (!nsOSFile::ExistsFile(sDumpToolPath))
    return nsStatus(nsFmt("MiniDumpTool.exe not found in '{}'", sDumpToolPath));

  nsProcessOptions procOpt;
  procOpt.m_sProcess = sDumpToolPath;
  procOpt.m_Arguments.PushBack("-PID");
  procOpt.AddArgument("{}", nsProcess::GetCurrentProcessID());
  procOpt.m_Arguments.PushBack("-f");
  procOpt.m_Arguments.PushBack(sDumpFile);

  if ((opt_FullCrashDumps.GetOptionValue(nsCommandLineOption::LogMode::Always) && dumpTypeOverride == nsDumpType::Auto) || dumpTypeOverride == nsDumpType::MiniDumpWithFullMemory)
  {
    // forward the '-fullcrashdumps' command line argument
    procOpt.AddArgument("-fullcrashdumps");
  }

  nsProcessGroup proc;
  if (proc.Launch(procOpt).Failed())
    return nsStatus(nsFmt("Failed to launch '{}'", sDumpToolPath));

  if (proc.WaitToFinish().Failed())
    return nsStatus("Waiting for MiniDumpTool to finish failed.");

  return nsStatus(NS_SUCCESS);

#  else
  return nsStatus("Not implemented on UPW");
#  endif
}

#endif
