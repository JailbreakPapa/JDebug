#pragma once

#if NS_ENABLED(NS_SUPPORTS_PROCESSES)
#  include <Foundation/System/Process.h>

/// \brief Process groups are used to tie multiple processes together and ensure they get terminated either on demand or when the
/// application crashes
///
/// On Windows when an nsProcessGroup instance is destroyed (either normally or due to a crash), all processes that have
/// been added to the group will be terminated by the OS. Other operating systems do not provide the terminate on crash guarantee.
///
/// Only processes that were launched asynchronously and in a suspended state can be added to process groups.
/// They will be resumed by the group.
class NS_FOUNDATION_DLL nsProcessGroup
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsProcessGroup);

public:
  /// \brief Creates a process group. The name is only used for debugging purposes.
  nsProcessGroup(nsStringView sGroupName = {});
  ~nsProcessGroup();

  /// \brief Launches a new process in the group.
  nsResult Launch(const nsProcessOptions& opt);

  /// \brief Waits for all the processes in the group to terminate.
  ///
  /// Returns NS_SUCCESS only if all processes have shut down.
  /// In all other cases, e.g. if the optional timeout is reached,
  /// NS_FAILURE is returned.
  nsResult WaitToFinish(nsTime timeout = nsTime::MakeZero());

  /// \brief Tries to kill all processes associated with this group.
  ///
  /// Sends a kill command to all processes and then waits indefinitely for them to terminate.
  /// Note: iForcedExitCode is only supported on Windows.
  nsResult TerminateAll(nsInt32 iForcedExitCode = -2);

  /// \brief Returns the container holding all processes of this group.
  ///
  /// This can be used to query per-process information such as exit codes.
  const nsHybridArray<nsProcess, 8>& GetProcesses() const;

private:
  nsUniquePtr<struct nsProcessGroupImpl> m_pImpl;

  nsHybridArray<nsProcess, 8> m_Processes;
};
#endif
