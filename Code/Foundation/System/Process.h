#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringView.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/Delegate.h>
#include <Foundation/Types/UniquePtr.h>

using nsOsProcessHandle = void*;
using nsOsProcessID = nsUInt32;

#if NS_ENABLED(NS_SUPPORTS_PROCESSES)
enum class nsProcessState
{
  NotStarted,
  Running,
  Finished
};

/// \brief Options that describe how to run an external process
struct NS_FOUNDATION_DLL nsProcessOptions
{
  /// Path to the binary to launch
  nsString m_sProcess;

  /// Custom working directory for the launched process. If empty, inherits the CWD from the parent process.
  nsString m_sWorkingDirectory;

  /// Arguments to pass to the process. Strings that contain spaces will be wrapped in quotation marks automatically
  nsHybridArray<nsString, 8> m_Arguments;

  /// If set to true, command line tools will not show their console window, but execute in the background
  bool m_bHideConsoleWindow = true;

  /// If set, stdout will be captured and this function called on a separate thread. Requires bWaitForResult to be true.
  nsDelegate<void(nsStringView)> m_onStdOut;

  /// If set, stderr will be captured and this function called on a separate thread. Requires bWaitForResult to be true.
  nsDelegate<void(nsStringView)> m_onStdError;

  /// \brief Appends a formatted argument to m_Arguments
  ///
  /// This can be useful if a complex command needs to be added as a single argument.
  /// Ie. since arguments with spaces will be wrapped in quotes, it can make a difference
  /// whether a complex parameter is added as one or multiple arguments.
  void AddArgument(const nsFormatString& arg);

  /// \brief Overload of AddArgument(nsFormatString) for convenience.
  template <typename... ARGS>
  void AddArgument(nsStringView sFormat, ARGS&&... args)
  {
    AddArgument(nsFormatStringImpl<ARGS...>(sFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Takes a full command line and appends it as individual arguments by splitting it along white-space and quotation marks.
  ///
  /// Brief, use this, if arguments are already pre-built as a full command line.
  void AddCommandLine(nsStringView sCmdLine);

  /// \brief Builds the command line from the process arguments and appends it to \a out_sCmdLine.
  void BuildCommandLineString(nsStringBuilder& out_sCmdLine) const;
};

/// \brief Flags for nsProcess::Launch()
struct nsProcessLaunchFlags
{
  using StorageType = nsUInt32;

  enum Enum
  {
    None = 0,
    Detached = NS_BIT(0),  ///< The process will be detached right after launch, as if nsProcess::Detach() was called.
    Suspended = NS_BIT(1), ///< The process will be launched in a suspended state. Call nsProcess::ResumeSuspended() to unpause it.
    Default = None
  };

  struct Bits
  {
    StorageType Detached : 1;
    StorageType Suspended : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsProcessLaunchFlags);

/// \brief Provides functionality to launch other processes
class NS_FOUNDATION_DLL nsProcess
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsProcess);

public:
  nsProcess();
  nsProcess(nsProcess&& rhs);

  /// \brief Upon destruction the running process will be terminated.
  ///
  /// Use Detach() to prevent the termination of the launched process.
  ///
  /// \sa Terminate()
  /// \sa Detach()
  ~nsProcess();

  /// \brief Launches the specified process and waits for it to finish.
  static nsResult Execute(const nsProcessOptions& opt, nsInt32* out_pExitCode = nullptr);

  /// \brief Launches the specified process asynchronously.
  ///
  /// When the function returns, the process is typically starting or running.
  /// Call WaitToFinish() to wait for the process to shutdown or Terminate() to kill it.
  ///
  /// \sa nsProcessLaunchFlags
  nsResult Launch(const nsProcessOptions& opt, nsBitflags<nsProcessLaunchFlags> launchFlags = nsProcessLaunchFlags::None);

  /// \brief Resumes a process that was launched in a suspended state. Returns NS_FAILURE if the process has not been launched or already
  /// resumed.
  nsResult ResumeSuspended();

  /// \brief Waits the given amount of time for the previously launched process to finish.
  ///
  /// Pass in nsTime::MakeZero() to wait indefinitely.
  /// Returns NS_FAILURE, if the process did not finish within the given time.
  ///
  /// \note Asserts that the nsProcess instance was used to successfully launch a process before.
  nsResult WaitToFinish(nsTime timeout = nsTime::MakeZero());

  /// \brief Kills the detached process, if possible.
  nsResult Terminate();

  /// \brief Returns the exit code of the process. The exit code will be -0xFFFF as long as the process has not finished.
  nsInt32 GetExitCode() const;

  /// \brief Returns the running state of the process
  ///
  /// If the state is 'finished' the exit code (as returned by GetExitCode() ) will be updated.
  nsProcessState GetState() const;

  /// \brief Detaches the running process from the nsProcess instance.
  ///
  /// This means the nsProcess instance loses control over terminating the process or communicating with it.
  /// It also means that the process will keep running and not get terminated when the nsProcess instance is destroyed.
  void Detach();

  /// \brief Returns the OS specific handle to the process
  nsOsProcessHandle GetProcessHandle() const;

  /// \brief Returns the OS-specific process ID (PID)
  nsOsProcessID GetProcessID() const;

  /// \brief Returns OS-specific process ID (PID) for the calling process
  static nsOsProcessID GetCurrentProcessID();

private:
  void BuildFullCommandLineString(const nsProcessOptions& opt, nsStringView sProcess, nsStringBuilder& cmd) const;

  nsUniquePtr<struct nsProcessImpl> m_pImpl;

  // the default value is used by GetExitCode() to determine whether it has to be reevaluated
  mutable nsInt32 m_iExitCode = -0xFFFF;

  nsString m_sProcess;
  nsDelegate<void(nsStringView)> m_OnStdOut;
  nsDelegate<void(nsStringView)> m_OnStdError;
  mutable nsTime m_ProcessExited = nsTime::MakeZero();
};
#endif
