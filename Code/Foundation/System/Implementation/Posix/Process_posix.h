#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Logging/Log.h>
#include <Foundation/System/Process.h>
#include <Foundation/Threading/Thread.h>
#include <Foundation/Threading/ThreadUtils.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

WD_DEFINE_AS_POD_TYPE(struct pollfd);

namespace
{
  wdResult AddFdFlags(int fd, int addFlags)
  {
    int flags = fcntl(fd, F_GETFD);
    flags |= addFlags;
    if (fcntl(fd, F_SETFD, flags) != 0)
    {
      wdLog::Error("Failed to set flags on {}: {}", fd, errno);
      return WD_FAILURE;
    }
    return WD_SUCCESS;
  }
} // namespace

struct wdProcessImpl
{
  ~wdProcessImpl()
  {
    StopStreamWatcher();
  }

  pid_t m_childPid = -1;
  bool m_exitCodeAvailable = false;
  bool m_processSuspended = false;

  struct StdStreamInfo
  {
    int fd;
    wdDelegate<void(wdStringView)> callback;
  };
  wdHybridArray<StdStreamInfo, 2> m_streams;
  wdDynamicArray<wdStringBuilder> m_overflowBuffers;
  wdUniquePtr<wdOSThread> m_streamWatcherThread;
  int m_wakeupPipeReadEnd = -1;
  int m_wakeupPipeWriteEnd = -1;

  static void* StreamWatcherThread(void* context)
  {
    wdProcessImpl* self = reinterpret_cast<wdProcessImpl*>(context);
    char buffer[4096];

    wdHybridArray<struct pollfd, 3> pollfds;

    pollfds.PushBack({self->m_wakeupPipeReadEnd, POLLIN, 0});
    for (StdStreamInfo& stream : self->m_streams)
    {
      pollfds.PushBack({stream.fd, POLLIN, 0});
    }

    bool run = true;
    while (run)
    {
      int result = poll(pollfds.GetData(), pollfds.GetCount(), -1);
      if (result > 0)
      {
        // Result at index 0 is special and means there was a WakeUp
        if (pollfds[0].revents != 0)
        {
          run = false;
        }

        for (wdUInt32 i = 1; i < pollfds.GetCount(); ++i)
        {
          if (pollfds[i].revents != 0)
          {
            wdStringBuilder& overflowBuffer = self->m_overflowBuffers[i - 1];
            StdStreamInfo& stream = self->m_streams[i - 1];
            pollfds[i].revents = 0;
            while (true)
            {
              ssize_t numBytes = read(stream.fd, buffer, WD_ARRAY_SIZE(buffer));
              if (numBytes < 0)
              {
                if (errno == EWOULDBLOCK)
                {
                  break;
                }
                wdLog::Error("Process Posix read error on {}: {}", stream.fd, errno);
                return nullptr;
              }
              if (numBytes == 0)
              {
                break;
              }

              const char* szCurrentPos = buffer;
              const char* szEndPos = buffer + numBytes;
              while (szCurrentPos < szEndPos)
              {
                const char* szFound = wdStringUtils::FindSubString(szCurrentPos, "\n", szEndPos);
                if (szFound)
                {
                  if (overflowBuffer.IsEmpty())
                  {
                    // If there is nothing in the overflow buffer this is a complete line and can be fired as is.
                    stream.callback(wdStringView(szCurrentPos, szFound + 1));
                  }
                  else
                  {
                    // We have data in the overflow buffer so this is the final part of a partial line so we need to complete and fire the overflow buffer.
                    overflowBuffer.Append(wdStringView(szCurrentPos, szFound + 1));
                    stream.callback(overflowBuffer);
                    overflowBuffer.Clear();
                  }
                  szCurrentPos = szFound + 1;
                }
                else
                {
                  // This is either the start or a middle segment of a line, append to overflow buffer.
                  overflowBuffer.Append(wdStringView(szCurrentPos, szEndPos));
                  szCurrentPos = szEndPos;
                }
              }
            }
          }
        }
      }
      else if (result < 0)
      {
        wdLog::Error("poll error {}", errno);
        break;
      }
    }

    for (wdUInt32 i = 0; i < self->m_streams.GetCount(); ++i)
    {
      wdStringBuilder& overflowBuffer = self->m_overflowBuffers[i];
      if (!overflowBuffer.IsEmpty())
      {
        self->m_streams[i].callback(overflowBuffer);
        overflowBuffer.Clear();
      }
    }

    return nullptr;
  }

  wdResult StartStreamWatcher()
  {
    int wakeupPipe[2] = {-1, -1};
    if (pipe(wakeupPipe) < 0)
    {
      wdLog::Error("Failed to setup wakeup pipe {}", errno);
      return WD_FAILURE;
    }
    else
    {
      m_wakeupPipeReadEnd = wakeupPipe[0];
      m_wakeupPipeWriteEnd = wakeupPipe[1];
      if (AddFdFlags(m_wakeupPipeReadEnd, O_NONBLOCK | O_CLOEXEC).Failed() ||
          AddFdFlags(m_wakeupPipeWriteEnd, O_NONBLOCK | O_CLOEXEC).Failed())
      {
        close(m_wakeupPipeReadEnd);
        m_wakeupPipeReadEnd = -1;
        close(m_wakeupPipeWriteEnd);
        m_wakeupPipeWriteEnd = -1;
        return WD_FAILURE;
      }
    }

    m_streamWatcherThread = WD_DEFAULT_NEW(wdOSThread, &StreamWatcherThread, this, "StdStrmWtch");
    m_streamWatcherThread->Start();

    return WD_SUCCESS;
  }

  void StopStreamWatcher()
  {
    if (m_streamWatcherThread)
    {
      char c = 0;
      WD_IGNORE_UNUSED(write(m_wakeupPipeWriteEnd, &c, 1));
      m_streamWatcherThread->Join();
      m_streamWatcherThread = nullptr;
    }
    close(m_wakeupPipeReadEnd);
    close(m_wakeupPipeWriteEnd);
    m_wakeupPipeReadEnd = -1;
    m_wakeupPipeWriteEnd = -1;
  }

  void AddStream(int fd, const wdDelegate<void(wdStringView)>& callback)
  {
    m_streams.PushBack({fd, callback});
    m_overflowBuffers.SetCount(m_streams.GetCount());
  }

  static wdResult StartChildProcess(const wdProcessOptions& opt, pid_t& outPid, bool suspended, int& outStdOutFd, int& outStdErrFd)
  {
    int stdoutPipe[2] = {-1, -1};
    int stderrPipe[2] = {-1, -1};

    if (opt.m_onStdOut.IsValid())
    {
      if (pipe(stdoutPipe) < 0)
      {
        return WD_FAILURE;
      }
    }

    if (opt.m_onStdError.IsValid())
    {
      if (pipe(stderrPipe) < 0)
      {
        return WD_FAILURE;
      }
    }

    pid_t childPid = fork();
    if (childPid < 0)
    {
      return WD_FAILURE;
    }

    if (childPid == 0) // We are the child
    {
      if (suspended)
      {
        if (raise(SIGSTOP) < 0)
        {
          _exit(-1);
        }
      }

      if (opt.m_bHideConsoleWindow == true)
      {
        // Redirect STDIN to /dev/null
        int stdinReplace = open("/dev/null", O_RDONLY);
        dup2(stdinReplace, STDIN_FILENO);
        close(stdinReplace);

        if (!opt.m_onStdOut.IsValid())
        {
          int stdoutReplace = open("/dev/null", O_WRONLY);
          dup2(stdoutReplace, STDOUT_FILENO);
          close(stdoutReplace);
        }

        if (!opt.m_onStdError.IsValid())
        {
          int stderrReplace = open("/dev/null", O_WRONLY);
          dup2(stderrReplace, STDERR_FILENO);
          close(stderrReplace);
        }
      }
      else
      {
        // TODO: Launch a x-terminal-emulator with the command and somehow redirect STDOUT, etc?
        WD_ASSERT_NOT_IMPLEMENTED;
      }

      if (opt.m_onStdOut.IsValid())
      {
        close(stdoutPipe[0]);               // We don't need the read end of the pipe in the child process
        dup2(stdoutPipe[1], STDOUT_FILENO); // redirect the write end to STDOUT
        close(stdoutPipe[1]);
      }

      if (opt.m_onStdError.IsValid())
      {
        close(stderrPipe[0]);               // We don't need the read end of the pipe in the child process
        dup2(stderrPipe[1], STDERR_FILENO); // redirect the write end to STDERR
        close(stderrPipe[1]);
      }

      wdHybridArray<char*, 9> args;

      for (const wdString& arg : opt.m_Arguments)
      {
        args.PushBack(const_cast<char*>(arg.GetData()));
      }
      args.PushBack(nullptr);

      if (!opt.m_sWorkingDirectory.IsEmpty())
      {
        if (chdir(opt.m_sWorkingDirectory.GetData()) < 0)
        {
          _exit(-1); // Failed to change working directory
        }
      }

      if (execv(opt.m_sProcess.GetData(), args.GetData()) < 0)
      {
        _exit(-1);
      }
    }
    else
    {
      outPid = childPid;

      if (opt.m_onStdOut.IsValid())
      {
        close(stdoutPipe[1]); // Don't need the write end in the parent process
        outStdOutFd = stdoutPipe[0];
      }

      if (opt.m_onStdError.IsValid())
      {
        close(stderrPipe[1]); // Don't need the write end in the parent process
        outStdErrFd = stderrPipe[0];
      }
    }

    return WD_SUCCESS;
  }
};

wdProcess::wdProcess()
{
  m_pImpl = WD_DEFAULT_NEW(wdProcessImpl);
}

wdProcess::~wdProcess()
{
  if (GetState() == wdProcessState::Running)
  {
    wdLog::Dev("Process still running - terminating '{}'", m_sProcess);

    Terminate().IgnoreResult();
  }

  // Explicitly clear the implementation here so that member
  // state (e.g. delegates) used by the impl survives the implementation.
  m_pImpl.Clear();
}

wdResult wdProcess::Execute(const wdProcessOptions& opt, wdInt32* out_iExitCode /*= nullptr*/)
{
  pid_t childPid = 0;
  int stdoutFd = -1;
  int stderrFd = -1;
  if (wdProcessImpl::StartChildProcess(opt, childPid, false, stdoutFd, stderrFd).Failed())
  {
    return WD_FAILURE;
  }

  wdProcessImpl impl;
  if (stdoutFd >= 0)
  {
    impl.AddStream(stdoutFd, opt.m_onStdOut);
  }

  if (stderrFd >= 0)
  {
    impl.AddStream(stderrFd, opt.m_onStdError);
  }

  if (stdoutFd >= 0 || stderrFd >= 0)
  {
    if (impl.StartStreamWatcher().Failed())
    {
      return WD_FAILURE;
    }
  }

  int childStatus = -1;
  pid_t waitedPid = waitpid(childPid, &childStatus, 0);
  if (waitedPid < 0)
  {
    return WD_FAILURE;
  }
  if (out_iExitCode != nullptr)
  {
    if (WIFEXITED(childStatus))
    {
      *out_iExitCode = WEXITSTATUS(childStatus);
    }
    else
    {
      *out_iExitCode = -1;
    }
  }
  return WD_SUCCESS;
}

wdResult wdProcess::Launch(const wdProcessOptions& opt, wdBitflags<wdProcessLaunchFlags> launchFlags /*= wdProcessLaunchFlags::None*/)
{
  WD_ASSERT_DEV(m_pImpl->m_childPid == -1, "Can not reuse an instance of wdProcess");

  int stdoutFd = -1;
  int stderrFd = -1;

  if (wdProcessImpl::StartChildProcess(opt, m_pImpl->m_childPid, launchFlags.IsSet(wdProcessLaunchFlags::Suspended), stdoutFd, stderrFd).Failed())
  {
    return WD_FAILURE;
  }

  m_pImpl->m_exitCodeAvailable = false;
  m_pImpl->m_processSuspended = launchFlags.IsSet(wdProcessLaunchFlags::Suspended);

  if (stdoutFd >= 0)
  {
    m_pImpl->AddStream(stdoutFd, opt.m_onStdOut);
  }

  if (stderrFd >= 0)
  {
    m_pImpl->AddStream(stderrFd, opt.m_onStdError);
  }

  if (stdoutFd >= 0 || stderrFd >= 0)
  {
    if (m_pImpl->StartStreamWatcher().Failed())
    {
      return WD_FAILURE;
    }
  }

  if (launchFlags.IsSet(wdProcessLaunchFlags::Detached))
  {
    Detach();
  }

  return WD_SUCCESS;
}

wdResult wdProcess::ResumeSuspended()
{
  if (m_pImpl->m_childPid < 0 || !m_pImpl->m_processSuspended)
  {
    return WD_FAILURE;
  }

  if (kill(m_pImpl->m_childPid, SIGCONT) < 0)
  {
    return WD_FAILURE;
  }
  m_pImpl->m_processSuspended = false;
  return WD_SUCCESS;
}

wdResult wdProcess::WaitToFinish(wdTime timeout /*= wdTime::Zero()*/)
{
  int childStatus = 0;
  WD_SCOPE_EXIT(m_pImpl->StopStreamWatcher());

  if (timeout.IsZero())
  {
    if (waitpid(m_pImpl->m_childPid, &childStatus, 0) < 0)
    {
      return WD_FAILURE;
    }
  }
  else
  {
    int waitResult = 0;
    wdTime startWait = wdTime::Now();
    while (true)
    {
      waitResult = waitpid(m_pImpl->m_childPid, &childStatus, WNOHANG);
      if (waitResult < 0)
      {
        return WD_FAILURE;
      }
      if (waitResult > 0)
      {
        break;
      }
      wdTime timeSpent = wdTime::Now() - startWait;
      if (timeSpent > timeout)
      {
        return WD_FAILURE;
      }
      wdThreadUtils::Sleep(wdMath::Min(wdTime::Milliseconds(100.0), timeout - timeSpent));
    }
  }

  if (WIFEXITED(childStatus))
  {
    m_iExitCode = WEXITSTATUS(childStatus);
  }
  else
  {
    m_iExitCode = -1;
  }
  m_pImpl->m_exitCodeAvailable = true;

  return WD_SUCCESS;
}

wdResult wdProcess::Terminate()
{
  if (m_pImpl->m_childPid == -1)
  {
    return WD_FAILURE;
  }

  WD_SCOPE_EXIT(m_pImpl->StopStreamWatcher());

  if (kill(m_pImpl->m_childPid, SIGKILL) < 0)
  {
    if (errno != ESRCH) // ESRCH = Process does not exist
    {
      return WD_FAILURE;
    }
  }
  m_pImpl->m_exitCodeAvailable = true;
  m_iExitCode = -1;

  return WD_SUCCESS;
}

wdProcessState wdProcess::GetState() const
{
  if (m_pImpl->m_childPid == -1)
  {
    return wdProcessState::NotStarted;
  }

  if (m_pImpl->m_exitCodeAvailable)
  {
    return wdProcessState::Finished;
  }

  int childStatus = -1;
  int waitResult = waitpid(m_pImpl->m_childPid, &childStatus, WNOHANG);
  if (waitResult > 0)
  {
    m_iExitCode = WEXITSTATUS(childStatus);
    m_pImpl->m_exitCodeAvailable = true;

    m_pImpl->StopStreamWatcher();

    return wdProcessState::Finished;
  }

  return wdProcessState::Running;
}

void wdProcess::Detach()
{
  m_pImpl->m_childPid = -1;
}

wdOsProcessHandle wdProcess::GetProcessHandle() const
{
  WD_ASSERT_DEV(false, "There is no process handle on posix");
  return nullptr;
}

wdOsProcessID wdProcess::GetProcessID() const
{
  WD_ASSERT_DEV(m_pImpl->m_childPid != -1, "No ProcessID available");
  return m_pImpl->m_childPid;
}

wdOsProcessID wdProcess::GetCurrentProcessID()
{
  return getpid();
}
