#pragma once

#include <Foundation/Basics.h>

#include <Foundation/Basics/Platform/Win/MinWindows.h>
#include <Foundation/Types/Status.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
extern "C"
{
  struct _EXCEPTION_POINTERS;
}
#endif

/// \brief Functionality for writing process mini-dumps (callstacks, memory snapshots, etc)
struct NS_FOUNDATION_DLL nsMiniDumpUtils
{

  /// \brief Specifies the dump mode that is written.
  enum class nsDumpType
  {
    Auto,                  ///< Uses the setting specified globally through the command line.
    MiniDump,              ///< Saves a mini-dump without full memory, regardless of this application's command line flag '-fullcrashdumps'.
    MiniDumpWithFullMemory ///< Saves a mini-dump with full memory, regardless of this application's command line flag '-fullcrashdumps'.
  };

  /// \brief Tries to write a mini-dump for the external process with the given process ID.
  ///
  /// \sa WriteProcessMiniDump()
  static nsStatus WriteExternalProcessMiniDump(nsStringView sDumpFile, nsUInt32 uiProcessID, nsDumpType dumpTypeOverride = nsDumpType::Auto);

  /// \brief Tries to launch ns's 'MiniDumpTool' to write a mini-dump for THIS process (the recommended way when an application is crashing).
  ///
  /// \note On Windows: The command line option '-fullcrashdumps' is passed if either set in this application's command line or if overridden through dumpTypeOverride = nsDumpType::MiniDumpWithFullMemory.
  static nsStatus LaunchMiniDumpTool(nsStringView sDumpFile, nsDumpType dumpTypeOverride = nsDumpType::Auto);

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
  /// \brief Windows-specific implementation for writing a mini-dump of the running process.
  ///
  /// \sa WriteProcessMiniDump()
  static nsStatus WriteOwnProcessMiniDump(nsStringView sDumpFile, struct _EXCEPTION_POINTERS* pExceptionInfo, nsDumpType dumpTypeOverride = nsDumpType::Auto);

  /// \brief Given a process ID this function tries to get a HANDLE to the process with the necessary access rights to write a mini-dump.
  static nsMinWindows::HANDLE GetProcessHandleWithNecessaryRights(nsUInt32 uiProcessID);

  /// \brief Windows-specific implementation for writing a mini-dump of another process.
  ///
  /// \sa WriteProcessMiniDump()
  static nsStatus WriteExternalProcessMiniDump(nsStringView sDumpFile, nsUInt32 uiProcessID, nsMinWindows::HANDLE hProcess, nsDumpType dumpTypeOverride = nsDumpType::Auto);

  /// \brief Windows-specific implementation for writing a mini-dump of the running process.
  ///
  /// \note On Windows: A crash-dump with a full memory capture is made if either this application's command line option '-fullcrashdumps' is specified or if that setting is overridden through dumpTypeOverride = nsDumpType::MiniDumpWithFullMemory.
  static nsStatus WriteProcessMiniDump(nsStringView sDumpFile, nsUInt32 uiProcessID, nsMinWindows::HANDLE hProcess, struct _EXCEPTION_POINTERS* pExceptionInfo, nsDumpType dumpTypeOverrideType = nsDumpType::Auto);

#endif
};
