#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringView.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Delegate.h>
#include <Foundation/Types/UniquePtr.h>

typedef void* wdOsProcessHandle;
typedef wdUInt32 wdOsProcessID;

#if WD_ENABLED(WD_SUPPORTS_PROCESSES)
enum class wdProcessState
{
  NotStarted,
  Running,
  Finished
};

/// \brief Options that describe how to run an external process
struct WD_FOUNDATION_DLL wdProcessOptions
{
  /// Path to the binary to launch
  wdString m_sProcess;

  /// Custom working directory for the launched process. If empty, inherits the CWD from the parent process.
  wdString m_sWorkingDirectory;

  /// Arguments to pass to the process. Strings that contain spaces will be wrapped in quotation marks automatically
  wdHybridArray<wdString, 8> m_Arguments;

  /// If set to true, command line tools will not show their console window, but execute in the background
  bool m_bHideConsoleWindow = true;

  /// If set, stdout will be captured and this function called on a separate thread. Requires bWaitForResult to be true.
  wdDelegate<void(wdStringView)> m_onStdOut;

  /// If set, stderr will be captured and this function called on a separate thread. Requires bWaitForResult to be true.
  wdDelegate<void(wdStringView)> m_onStdError;

  /// \brief Appends a formatted argument to m_Arguments
  ///
  /// This can be useful if a complex command needs to be added as a single argument.
  /// Ie. since arguments with spaces will be wrapped in quotes, it can make a difference
  /// whether a complex parameter is added as one or multiple arguments.
  void AddArgument(const wdFormatString& arg);

  /// \brief Overload of AddArgument(wdFormatString) for convenience.
  template <typename... ARGS>
  void AddArgument(const char* szFormat, ARGS&&... args)
  {
    AddArgument(wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Takes a full command line and appends it as individual arguments by splitting it along white-space and quotation marks.
  ///
  /// Brief, use this, if arguments are already pre-built as a full command line.
  void AddCommandLine(const char* szCmdLine);

  /// \brief Builds the command line from the process arguments and appends it to \a out_sCmdLine.
  void BuildCommandLineString(wdStringBuilder& out_sCmdLine) const;
};

/// \brief Flags for wdProcess::Launch()
struct wdProcessLaunchFlags
{
  typedef wdUInt32 StorageType;

  enum Enum
  {
    None = 0,
    Detached = WD_BIT(0),  ///< The process will be detached right after launch, as if wdProcess::Detach() was called.
    Suspended = WD_BIT(1), ///< The process will be launched in a suspended state. Call wdProcess::ResumeSuspended() to unpause it.
    Default = None
  };

  struct Bits
  {
    StorageType Detached : 1;
    StorageType Suspended : 1;
  };
};

WD_DECLARE_FLAGS_OPERATORS(wdProcessLaunchFlags);

/// \brief Provides functionality to launch other processes
class WD_FOUNDATION_DLL wdProcess
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdProcess);

public:
  wdProcess();
  wdProcess(wdProcess&& rhs);

  /// \brief Upon destruction the running process will be terminated.
  ///
  /// Use Detach() to prevent the termination of the launched process.
  ///
  /// \sa Terminate()
  /// \sa Detach()
  ~wdProcess();

  /// \brief Launches the specified process and waits for it to finish.
  static wdResult Execute(const wdProcessOptions& opt, wdInt32* out_pExitCode = nullptr);

  /// \brief Launches the specified process asynchronously.
  ///
  /// When the function returns, the process is typically starting or running.
  /// Call WaitToFinish() to wait for the process to shutdown or Terminate() to kill it.
  ///
  /// \sa wdProcessLaunchFlags
  wdResult Launch(const wdProcessOptions& opt, wdBitflags<wdProcessLaunchFlags> launchFlags = wdProcessLaunchFlags::None);

  /// \brief Resumes a process that was launched in a suspended state. Returns WD_FAILURE if the process has not been launched or already
  /// resumed.
  wdResult ResumeSuspended();

  /// \brief Waits the given amount of time for the previously launched process to finish.
  ///
  /// Pass in wdTime::Zero() to wait indefinitely.
  /// Returns WD_FAILURE, if the process did not finish within the given time.
  ///
  /// \note Asserts that the wdProcess instance was used to successfully launch a process before.
  wdResult WaitToFinish(wdTime timeout = wdTime::Zero());

  /// \brief Kills the detached process, if possible.
  wdResult Terminate();

  /// \brief Returns the exit code of the process. The exit code will be -0xFFFF as long as the process has not finished.
  wdInt32 GetExitCode() const;

  /// \brief Returns the running state of the process
  ///
  /// If the state is 'finished' the exit code (as returned by GetExitCode() ) will be updated.
  wdProcessState GetState() const;

  /// \brief Detaches the running process from the wdProcess instance.
  ///
  /// This means the wdProcess instance loses control over terminating the process or communicating with it.
  /// It also means that the process will keep running and not get terminated when the wdProcess instance is destroyed.
  void Detach();

  /// \brief Returns the OS specific handle to the process
  wdOsProcessHandle GetProcessHandle() const;

  /// \brief Returns the OS-specific process ID (PID)
  wdOsProcessID GetProcessID() const;

  /// \brief Returns OS-specific process ID (PID) for the calling process
  static wdOsProcessID GetCurrentProcessID();

private:
  void BuildFullCommandLineString(const wdProcessOptions& opt, const char* szProcess, wdStringBuilder& cmd) const;

  wdUniquePtr<struct wdProcessImpl> m_pImpl;

  // the default value is used by GetExitCode() to determine whether it has to be reevaluated
  mutable wdInt32 m_iExitCode = -0xFFFF;

  wdString m_sProcess;
  wdDelegate<void(wdStringView)> m_OnStdOut;
  wdDelegate<void(wdStringView)> m_OnStdError;
};
#endif
