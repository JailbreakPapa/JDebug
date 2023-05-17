#pragma once

#include <Foundation/Basics.h>

#include <Foundation/Basics/Platform/Win/MinWindows.h>
#include <Foundation/Types/Status.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
extern "C"
{
  struct _EXCEPTION_POINTERS;
}
#endif

/// \brief Functionality for writing process mini-dumps (callstacks, memory snapshots, etc)
struct WD_FOUNDATION_DLL wdMiniDumpUtils
{
  /// \brief Tries to write a mini-dump for the external process with the given process ID.
  ///
  /// \sa WriteProcessMiniDump()
  static wdStatus WriteExternalProcessMiniDump(const char* szDumpFile, wdUInt32 uiProcessID);

  /// \brief Tries to launch wd's 'MiniDumpTool' to write a mini-dump for THIS process (the recommended way when an application is crashing).
  ///
  /// \note On Windows: If the command line option '-fullcrashdumps' is specified, it is forwarded to the MiniDumpTool.
  static wdStatus LaunchMiniDumpTool(const char* szDumpFile);

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
  /// \brief Windows-specific implementation for writing a mini-dump of the running process.
  ///
  /// \sa WriteProcessMiniDump()
  static wdStatus WriteOwnProcessMiniDump(const char* szDumpFile, struct _EXCEPTION_POINTERS* pExceptionInfo);

  /// \brief Given a process ID this function tries to get a HANDLE to the process with the necessary access rights to write a mini-dump.
  static wdMinWindows::HANDLE GetProcessHandleWithNecessaryRights(wdUInt32 uiProcessID);

  /// \brief Windows-specific implementation for writing a mini-dump of another process.
  ///
  /// \sa WriteProcessMiniDump()
  static wdStatus WriteExternalProcessMiniDump(const char* szDumpFile, wdUInt32 uiProcessID, wdMinWindows::HANDLE pProcess);

  /// \brief Windows-specific implementation for writing a mini-dump of the running process.
  ///
  /// \note On Windows: If the command line option '-fullcrashdumps' is specified, a crash-dump with a full memory capture is made.
  static wdStatus WriteProcessMiniDump(
    const char* szDumpFile, wdUInt32 uiProcessID, wdMinWindows::HANDLE pProcess, struct _EXCEPTION_POINTERS* pExceptionInfo);

#endif
};
