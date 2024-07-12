#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS) && NS_ENABLED(NS_SUPPORTS_PROCESSES)

#  include <Foundation/Logging/Log.h>
#  include <Foundation/System/ProcessGroup.h>

struct nsProcessGroupImpl
{
  HANDLE m_hJobObject = INVALID_HANDLE_VALUE;
  HANDLE m_hCompletionPort = INVALID_HANDLE_VALUE;
  nsString m_sName;

  ~nsProcessGroupImpl();
  void Close();
  void Initialize();
};

nsProcessGroupImpl::~nsProcessGroupImpl()
{
  Close();
}

void nsProcessGroupImpl::Close()
{
  if (m_hJobObject != INVALID_HANDLE_VALUE)
  {
    CloseHandle(m_hJobObject);
    m_hJobObject = INVALID_HANDLE_VALUE;
  }
}

void nsProcessGroupImpl::Initialize()
{
  if (m_hJobObject == INVALID_HANDLE_VALUE)
  {
    m_hJobObject = CreateJobObjectW(nullptr, nullptr);

    if (m_hJobObject == nullptr || m_hJobObject == INVALID_HANDLE_VALUE)
    {
      nsLog::Error("Failed to create process group '{}' - {}", m_sName, nsArgErrorCode(GetLastError()));
      return;
    }

    // configure the job object such that it kill all processes once this job object is cleaned up
    // ie. either when all job object handles are closed, or the application crashes

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION exinfo = {};
    exinfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if (SetInformationJobObject(m_hJobObject, JobObjectExtendedLimitInformation, &exinfo, sizeof(exinfo)) == FALSE)
    {
      nsLog::Error("nsProcessGroup: failed to configure 'kill jobs on close' - '{}'", nsArgErrorCode(GetLastError()));
    }

    // the completion port is necessary to implement WaitToFinish()
    // see https://devblogs.microsoft.com/oldnewthing/20130405-00/?p=4743
    m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);

    JOBOBJECT_ASSOCIATE_COMPLETION_PORT Port;
    Port.CompletionKey = m_hJobObject;
    Port.CompletionPort = m_hCompletionPort;
    SetInformationJobObject(m_hJobObject, JobObjectAssociateCompletionPortInformation, &Port, sizeof(Port));
  }
}

nsProcessGroup::nsProcessGroup(nsStringView sGroupName)
{
  m_pImpl = NS_DEFAULT_NEW(nsProcessGroupImpl);
  m_pImpl->m_sName = sGroupName;
}

nsProcessGroup::~nsProcessGroup()
{
  TerminateAll().IgnoreResult();
}

nsResult nsProcessGroup::Launch(const nsProcessOptions& opt)
{
  m_pImpl->Initialize();

  nsProcess& process = m_Processes.ExpandAndGetRef();
  NS_SUCCEED_OR_RETURN(process.Launch(opt, nsProcessLaunchFlags::Suspended));

  if (AssignProcessToJobObject(m_pImpl->m_hJobObject, process.GetProcessHandle()) == FALSE)
  {
    nsLog::Error("Failed to add process to process group '{}' - {}", m_pImpl->m_sName, nsArgErrorCode(GetLastError()));
    m_Processes.PopBack();
    return NS_FAILURE;
  }

  if (process.ResumeSuspended().Failed())
  {
    nsLog::Error("Failed to resume the given process. Processes must be launched in a suspended state before adding them to process groups.");
    m_Processes.PopBack();
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsProcessGroup::WaitToFinish(nsTime timeout /*= nsTime::MakeZero()*/)
{
  if (m_pImpl->m_hJobObject == INVALID_HANDLE_VALUE)
    return NS_SUCCESS;

  // check if no new processes were launched, because waiting could end up in an infinite loop,
  // so don't even try in this case
  bool allProcessesGone = true;
  for (const nsProcess& p : m_Processes)
  {
    DWORD exitCode = 0;
    GetExitCodeProcess(p.GetProcessHandle(), &exitCode);
    if (exitCode == STILL_ACTIVE)
    {
      allProcessesGone = false;
      break;
    }
  }

  if (allProcessesGone)
  {
    // We need to wait for processes even if the job is done as the threads for the pipes are potentially still alive and lead to incomplete stdout / stderr output even though the process has exited.
    for (nsProcess& p : m_Processes)
    {
      p.WaitToFinish().IgnoreResult();
    }
    m_pImpl->Close();
    return NS_SUCCESS;
  }

  DWORD dwTimeout = INFINITE;

  if (timeout.IsPositive())
    dwTimeout = (DWORD)timeout.GetMilliseconds();
  else
    dwTimeout = INFINITE;

  DWORD CompletionCode;
  ULONG_PTR CompletionKey;
  LPOVERLAPPED Overlapped;

  nsTime tStart = nsTime::Now();

  while (true)
  {
    // ATTENTION !
    // If you are looking at a crash dump of ns this line will typically be at the top of the callstack.
    // That is because to write the crash dump an external process is called and this is where we are waiting for that process to finish.
    // To see the actual reason for the crash, locate the call to nsCrashHandlerFunc further down in the callstack.
    // The crashing code is usually the one calling that function.

    if (GetQueuedCompletionStatus(m_pImpl->m_hCompletionPort, &CompletionCode, &CompletionKey, &Overlapped, dwTimeout) == FALSE)
    {
      DWORD res = GetLastError();

      if (res != WAIT_TIMEOUT)
      {
        nsLog::Error("Failed to wait for process group '{}' - {}", m_pImpl->m_sName, nsArgErrorCode(res));
      }

      return NS_FAILURE;
    }

    // we got the expected result, all processes have finished
    if (((HANDLE)CompletionKey == m_pImpl->m_hJobObject && CompletionCode == JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO))
    {
      // We need to wait for processes even if the job is done as the threads for the pipes are potentially still alive and lead to incomplete stdout / stderr output even though the process has exited.
      for (nsProcess& p : m_Processes)
      {
        p.WaitToFinish().IgnoreResult();
      }

      m_pImpl->Close();
      return NS_SUCCESS;
    }

    // we got some different message, ignore this
    // however, we need to adjust our timeout

    if (timeout.IsPositive())
    {
      // subtract the time that we spent
      const nsTime now = nsTime::Now();
      timeout -= now - tStart;
      tStart = now;

      // the timeout has been reached
      if (timeout.IsZeroOrNegative())
      {
        return NS_FAILURE;
      }

      // otherwise try again, but with a reduced timeout
      dwTimeout = (DWORD)timeout.GetMilliseconds();
    }
  }
}

nsResult nsProcessGroup::TerminateAll(nsInt32 iForcedExitCode /*= -2*/)
{
  if (m_pImpl->m_hJobObject == INVALID_HANDLE_VALUE)
    return NS_SUCCESS;

  if (TerminateJobObject(m_pImpl->m_hJobObject, (UINT)iForcedExitCode) == FALSE)
  {
    nsLog::Error("Failed to terminate process group '{}' - {}", m_pImpl->m_sName, nsArgErrorCode(GetLastError()));
    return NS_FAILURE;
  }

  NS_SUCCEED_OR_RETURN(WaitToFinish());

  return NS_SUCCESS;
}

#endif
