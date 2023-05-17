#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Types/Bitflags.h>

/// \brief Helper class to manage the top level exception handler.
///
/// A default exception handler is provided but not set by default.
/// The default implementation will write the exception and callstack to the output
/// and create a memory dump using WriteDump that create a dump file in the folder
/// specified via SetExceptionHandler.

/// \brief This class allows to hook into the OS top-level exception handler to handle application crashes
///
/// Derive from this class to implement custom behavior. Call wdCrashHandler::SetCrashHandler() to
/// register which instance to use.
///
/// For typical use-cases use wdCrashHandler_WriteMiniDump::g_Instance.
class WD_FOUNDATION_DLL wdCrashHandler
{
public:
  wdCrashHandler();
  virtual ~wdCrashHandler();

  static void SetCrashHandler(wdCrashHandler* pHandler);
  static wdCrashHandler* GetCrashHandler();

  virtual void HandleCrash(void* pOsSpecificData) = 0;

private:
  static wdCrashHandler* s_pActiveHandler;
};

/// \brief A default implementation of wdCrashHandler that tries to write a mini-dump and prints the callstack.
///
/// To use it, call wdCrashHandler::SetCrashHandler(&wdCrashHandler_WriteMiniDump::g_Instance);
/// Do not forget to also specify the dump-file path, otherwise writing dump-files is skipped.
class WD_FOUNDATION_DLL wdCrashHandler_WriteMiniDump : public wdCrashHandler
{
public:
  static wdCrashHandler_WriteMiniDump g_Instance;

  struct PathFlags
  {
    typedef wdUInt8 StorageType;

    enum Enum
    {
      AppendDate = WD_BIT(0),      ///< Whether to append the current date to the crash-dump file (YYYY-MM-DD_HH-MM-SS)
      AppendSubFolder = WD_BIT(1), ///< Whether to append "CrashDump" as a sub-folder
      AppendPID = WD_BIT(2),       ///< Whether to append the process ID to the crash-dump file

      Default = AppendDate | AppendSubFolder | AppendPID
    };

    struct Bits
    {
      StorageType AppendDate : 1;
      StorageType AppendSubFolder : 1;
      StorageType AppendPID : 1;
    };
  };

public:
  wdCrashHandler_WriteMiniDump();

  /// \brief Sets the raw path for the dump-file to write
  void SetFullDumpFilePath(wdStringView sFullAbsDumpFilePath);

  /// \brief Sets the dump-file path to "{szAbsDirectoryPath}/{szAppName}_{cur-date}.tmp"
  void SetDumpFilePath(wdStringView sAbsDirectoryPath, wdStringView sAppName, wdBitflags<PathFlags> flags = PathFlags::Default);

  /// \brief Sets the dump-file path to "{wdOSFile::GetApplicationDirectory()}/{szAppName}_{cur-date}.tmp"
  void SetDumpFilePath(wdStringView sAppName, wdBitflags<PathFlags> flags = PathFlags::Default);

  virtual void HandleCrash(void* pOsSpecificData) override;

protected:
  virtual bool WriteOwnProcessMiniDump(void* pOsSpecificData);
  virtual void PrintStackTrace(void* pOsSpecificData);

  wdString m_sDumpFilePath;
};

WD_DECLARE_FLAGS_OPERATORS(wdCrashHandler_WriteMiniDump::PathFlags);
