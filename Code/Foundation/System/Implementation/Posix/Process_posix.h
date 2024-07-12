#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/System/Process.h>
#include <Foundation/Threading/Thread.h>
#include <Foundation/Threading/ThreadUtils.h>

#include <Foundation/System/SystemInformation.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef _NS_DEFINED_POLLFD_POD
#  define _NS_DEFINED_POLLFD_POD
NS_DEFINE_AS_POD_TYPE(struct pollfd);
#endif

class nsFd
{
public:
  nsFd() = default;
  nsFd(const nsFd&) = delete;
  nsFd(nsFd&& other)
  {
    m_fd = other.m_fd;
    other.m_fd = -1;
  }

  ~nsFd()
  {
    Close();
  }

  void Close()
  {
    if (m_fd != -1)
    {
      close(m_fd);
      m_fd = -1;
    }
  }

  bool IsValid() const
  {
    return m_fd >= 0;
  }

  void operator=(const nsFd&) = delete;
  void operator=(nsFd&& other)
  {
    Close();
    m_fd = other.m_fd;
    other.m_fd = -1;
  }

  void TakeOwnership(int fd)
  {
    Close();
    m_fd = fd;
  }

  int Borrow() const { return m_fd; }

  int Detach()
  {
    auto result = m_fd;
    m_fd = -1;
    return result;
  }

  nsResult AddFlags(int addFlags)
  {
    if (m_fd < 0)
      return NS_FAILURE;

    if (addFlags & O_CLOEXEC)
    {
      int flags = fcntl(m_fd, F_GETFD);
      flags |= FD_CLOEXEC;
      if (fcntl(m_fd, F_SETFD, flags) != 0)
      {
        nsLog::Error("Failed to set flags on {}: {}", m_fd, errno);
        return NS_FAILURE;
      }
      addFlags &= ~O_CLOEXEC;
    }

    if (addFlags)
    {
      int flags = fcntl(m_fd, F_GETFL);
      flags |= addFlags;
      if (fcntl(m_fd, F_SETFD, flags) != 0)
      {
        nsLog::Error("Failed to set flags on {}: {}", m_fd, errno);
        return NS_FAILURE;
      }
    }

    return NS_SUCCESS;
  }

  static nsResult MakePipe(nsFd (&fds)[2], int flags = 0)
  {
    fds[0].Close();
    fds[1].Close();
#if NS_ENABLED(NS_USE_LINUX_POSIX_EXTENSIONS)
    if (pipe2((int*)fds, flags) != 0)
    {
      return NS_FAILURE;
    }
#else
    if (pipe((int*)fds) != 0)
    {
      return NS_FAILURE;
    }
    if (flags != 0 && (fds[0].AddFlags(flags).Failed() || fds[1].AddFlags(flags).Failed()))
    {
      fds[0].Close();
      fds[1].Close();
      return NS_FAILURE;
    }
#endif
    return NS_SUCCESS;
  }

private:
  int m_fd = -1;
};

namespace
{
  struct ProcessStartupError
  {
    enum class Type : nsUInt32
    {
      FailedToChangeWorkingDirectory = 0,
      FailedToExecv = 1
    };

    Type type;
    int errorCode;
  };
} // namespace


struct nsProcessImpl
{
  ~nsProcessImpl()
  {
    StopStreamWatcher();
  }

  pid_t m_childPid = -1;
  bool m_exitCodeAvailable = false;
  bool m_processSuspended = false;

  struct StdStreamInfo
  {
    nsFd fd;
    nsDelegate<void(nsStringView)> callback;
  };
  nsHybridArray<StdStreamInfo, 2> m_streams;
  nsDynamicArray<nsStringBuilder> m_overflowBuffers;
  nsUniquePtr<nsOSThread> m_streamWatcherThread;
  nsFd m_wakeupPipeReadEnd;
  nsFd m_wakeupPipeWriteEnd;

  static void* StreamWatcherThread(void* context)
  {
    nsProcessImpl* self = reinterpret_cast<nsProcessImpl*>(context);
    char buffer[4096];

    nsHybridArray<struct pollfd, 3> pollfds;

    pollfds.PushBack({self->m_wakeupPipeReadEnd.Borrow(), POLLIN, 0});
    for (StdStreamInfo& stream : self->m_streams)
    {
      pollfds.PushBack({stream.fd.Borrow(), POLLIN, 0});
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

        for (nsUInt32 i = 1; i < pollfds.GetCount(); ++i)
        {
          if (pollfds[i].revents & POLLIN)
          {
            nsStringBuilder& overflowBuffer = self->m_overflowBuffers[i - 1];
            StdStreamInfo& stream = self->m_streams[i - 1];
            while (true)
            {
              ssize_t numBytes = read(stream.fd.Borrow(), buffer, NS_ARRAY_SIZE(buffer));
              if (numBytes < 0)
              {
                if (errno == EWOULDBLOCK)
                {
                  break;
                }
                nsLog::Error("Process Posix read error on {}: {}", stream.fd.Borrow(), errno);
                return nullptr;
              }

              const char* szCurrentPos = buffer;
              const char* szEndPos = buffer + numBytes;
              while (szCurrentPos < szEndPos)
              {
                const char* szFound = nsStringUtils::FindSubString(szCurrentPos, "\n", szEndPos);
                if (szFound)
                {
                  if (overflowBuffer.IsEmpty())
                  {
                    // If there is nothing in the overflow buffer this is a complete line and can be fired as is.
                    stream.callback(nsStringView(szCurrentPos, szFound + 1));
                  }
                  else
                  {
                    // We have data in the overflow buffer so this is the final part of a partial line so we need to complete and fire the overflow buffer.
                    overflowBuffer.Append(nsStringView(szCurrentPos, szFound + 1));
                    stream.callback(overflowBuffer);
                    overflowBuffer.Clear();
                  }
                  szCurrentPos = szFound + 1;
                }
                else
                {
                  // This is either the start or a middle segment of a line, append to overflow buffer.
                  overflowBuffer.Append(nsStringView(szCurrentPos, szEndPos));
                  szCurrentPos = szEndPos;
                }
              }

              if (numBytes < NS_ARRAY_SIZE(buffer))
              {
                break;
              }
            }
          }
          pollfds[i].revents = 0;
        }
      }
      else if (result < 0)
      {
        nsLog::Error("poll error {}", errno);
        break;
      }
    }

    for (nsUInt32 i = 0; i < self->m_streams.GetCount(); ++i)
    {
      nsStringBuilder& overflowBuffer = self->m_overflowBuffers[i];
      if (!overflowBuffer.IsEmpty())
      {
        self->m_streams[i].callback(overflowBuffer);
        overflowBuffer.Clear();
      }

      self->m_streams[i].fd.Close();
    }

    return nullptr;
  }

  nsResult StartStreamWatcher()
  {
    nsFd wakeupPipe[2];
    if (nsFd::MakePipe(wakeupPipe, O_NONBLOCK | O_CLOEXEC).Failed())
    {
      nsLog::Error("Failed to setup wakeup pipe {}", errno);
      return NS_FAILURE;
    }
    else
    {
      m_wakeupPipeReadEnd = std::move(wakeupPipe[0]);
      m_wakeupPipeWriteEnd = std::move(wakeupPipe[1]);
    }

    m_streamWatcherThread = NS_DEFAULT_NEW(nsOSThread, &StreamWatcherThread, this, "StdStrmWtch");
    m_streamWatcherThread->Start();

    return NS_SUCCESS;
  }

  void StopStreamWatcher()
  {
    if (m_streamWatcherThread)
    {
      char c = 0;
      NS_IGNORE_UNUSED(write(m_wakeupPipeWriteEnd.Borrow(), &c, 1));
      m_streamWatcherThread->Join();
      m_streamWatcherThread = nullptr;
    }
    m_wakeupPipeReadEnd.Close();
    m_wakeupPipeWriteEnd.Close();
  }

  void AddStream(nsFd fd, const nsDelegate<void(nsStringView)>& callback)
  {
    m_streams.PushBack({std::move(fd), callback});
    m_overflowBuffers.SetCount(m_streams.GetCount());
  }

  nsUInt32 GetNumStreams() const { return m_streams.GetCount(); }

  static nsResult StartChildProcess(const nsProcessOptions& opt, pid_t& outPid, bool suspended, nsFd& outStdOutFd, nsFd& outStdErrFd)
  {
    nsFd stdoutPipe[2];
    nsFd stderrPipe[2];
    nsFd startupErrorPipe[2];

    nsStringBuilder executablePath = opt.m_sProcess;
    nsFileStats stats;
    if (!opt.m_sProcess.IsAbsolutePath())
    {
      executablePath = nsOSFile::GetCurrentWorkingDirectory();
      executablePath.AppendPath(opt.m_sProcess);
    }

    if (nsOSFile::GetFileStats(executablePath, stats).Failed() || stats.m_bIsDirectory)
    {
      nsHybridArray<char, 512> confPath;
      auto envPATH = getenv("PATH");
      if (envPATH == nullptr) // if no PATH environment variable is available, we need to fetch the system default;
      {
#if _POSIX_C_SOURCE >= 2 || _XOPEN_SOURCE
        size_t confPathSize = confstr(_CS_PATH, nullptr, 0);
        if (confPathSize > 0)
        {
          confPath.SetCountUninitialized(confPathSize);
          if (confstr(_CS_PATH, confPath.GetData(), confPath.GetCount()) == 0)
          {
            confPath.SetCountUninitialized(0);
          }
        }
#endif
        if (confPath.GetCount() == 0)
        {
          confPath.PushBack('\0');
        }
        envPATH = confPath.GetData();
      }

      nsStringView path = envPATH;
      nsHybridArray<nsStringView, 16> pathParts;
      path.Split(false, pathParts, ":");

      for (auto& pathPart : pathParts)
      {
        executablePath = pathPart;
        executablePath.AppendPath(opt.m_sProcess);
        if (nsOSFile::GetFileStats(executablePath, stats).Succeeded() && !stats.m_bIsDirectory)
        {
          break;
        }
        executablePath.Clear();
      }
    }

    if (executablePath.IsEmpty())
    {
      return NS_FAILURE;
    }

    if (opt.m_onStdOut.IsValid())
    {
      if (nsFd::MakePipe(stdoutPipe).Failed())
      {
        return NS_FAILURE;
      }
      if (stdoutPipe[0].AddFlags(O_NONBLOCK).Failed())
      {
        return NS_FAILURE;
      }
    }

    if (opt.m_onStdError.IsValid())
    {
      if (nsFd::MakePipe(stderrPipe).Failed())
      {
        return NS_FAILURE;
      }
      if (stderrPipe[0].AddFlags(O_NONBLOCK).Failed())
      {
        return NS_FAILURE;
      }
    }

    if (nsFd::MakePipe(startupErrorPipe, O_CLOEXEC).Failed())
    {
      return NS_FAILURE;
    }

    pid_t childPid = fork();
    if (childPid < 0)
    {
      return NS_FAILURE;
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
        NS_ASSERT_NOT_IMPLEMENTED;
      }

      if (opt.m_onStdOut.IsValid())
      {
        stdoutPipe[0].Close();                       // We don't need the read end of the pipe in the child process
        dup2(stdoutPipe[1].Borrow(), STDOUT_FILENO); // redirect the write end to STDOUT
        stdoutPipe[1].Close();
      }

      if (opt.m_onStdError.IsValid())
      {
        stderrPipe[0].Close();                       // We don't need the read end of the pipe in the child process
        dup2(stderrPipe[1].Borrow(), STDERR_FILENO); // redirect the write end to STDERR
        stderrPipe[1].Close();
      }

      startupErrorPipe[0].Close(); // we don't need the read end of the startup error pipe in the child process

      nsHybridArray<char*, 9> args;

      args.PushBack(const_cast<char*>(executablePath.GetData()));
      for (const nsString& arg : opt.m_Arguments)
      {
        args.PushBack(const_cast<char*>(arg.GetData()));
      }
      args.PushBack(nullptr);

      if (!opt.m_sWorkingDirectory.IsEmpty())
      {
        if (chdir(opt.m_sWorkingDirectory.GetData()) < 0)
        {
          auto err = ProcessStartupError{ProcessStartupError::Type::FailedToChangeWorkingDirectory, 0};
          NS_IGNORE_UNUSED(write(startupErrorPipe[1].Borrow(), &err, sizeof(err)));
          startupErrorPipe[1].Close();
          _exit(-1);
        }
      }

      if (execv(executablePath, args.GetData()) < 0)
      {
        auto err = ProcessStartupError{ProcessStartupError::Type::FailedToExecv, errno};
        NS_IGNORE_UNUSED(write(startupErrorPipe[1].Borrow(), &err, sizeof(err)));
        startupErrorPipe[1].Close();
        _exit(-1);
      }
    }
    else
    {
      startupErrorPipe[1].Close(); // We don't need the write end of the startup error pipe in the parent process
      stdoutPipe[1].Close();       // Don't need the write end in the parent process
      stderrPipe[1].Close();       // Don't need the write end in the parent process

      ProcessStartupError err = {};
      auto errSize = read(startupErrorPipe[0].Borrow(), &err, sizeof(err));
      startupErrorPipe[0].Close(); // we no longer need the read end of the startup error pipe

      // There are two possible cases here
      // Case 1: errSize is equal to 0, which means no error happened on the startupErrorPipe was closed during the execv call
      // Case 2: errSize > 0 in which case there was an error before the pipe was closed normally.
      if (errSize > 0)
      {
        NS_ASSERT_DEV(errSize == sizeof(err), "Child process should have written a full ProcessStartupError struct");
        switch (err.type)
        {
          case ProcessStartupError::Type::FailedToChangeWorkingDirectory:
            nsLog::Error("Failed to start process '{}' because the given working directory '{}' is invalid", opt.m_sProcess, opt.m_sWorkingDirectory);
            break;
          case ProcessStartupError::Type::FailedToExecv:
            nsLog::Error("Failed to exec when starting process '{}' the error code is '{}'", opt.m_sProcess, err.errorCode);
            break;
        }
        return NS_FAILURE;
      }

      outPid = childPid;

      if (opt.m_onStdOut.IsValid())
      {
        outStdOutFd = std::move(stdoutPipe[0]);
      }

      if (opt.m_onStdError.IsValid())
      {
        outStdErrFd = std::move(stderrPipe[0]);
      }
    }

    return NS_SUCCESS;
  }
};

nsProcess::nsProcess()
{
  m_pImpl = NS_DEFAULT_NEW(nsProcessImpl);
}

nsProcess::~nsProcess()
{
  if (GetState() == nsProcessState::Running)
  {
    nsLog::Dev("Process still running - terminating '{}'", m_sProcess);

    Terminate().IgnoreResult();
  }

  // Explicitly clear the implementation here so that member
  // state (e.g. delegates) used by the impl survives the implementation.
  m_pImpl.Clear();
}

nsResult nsProcess::Execute(const nsProcessOptions& opt, nsInt32* out_iExitCode /*= nullptr*/)
{
  pid_t childPid = 0;
  nsFd stdoutFd;
  nsFd stderrFd;
  if (nsProcessImpl::StartChildProcess(opt, childPid, false, stdoutFd, stderrFd).Failed())
  {
    return NS_FAILURE;
  }

  nsProcessImpl impl;
  if (stdoutFd.IsValid())
  {
    impl.AddStream(std::move(stdoutFd), opt.m_onStdOut);
  }

  if (stderrFd.IsValid())
  {
    impl.AddStream(std::move(stderrFd), opt.m_onStdError);
  }

  if (impl.GetNumStreams() > 0 && impl.StartStreamWatcher().Failed())
  {
    return NS_FAILURE;
  }

  int childStatus = -1;
  pid_t waitedPid = waitpid(childPid, &childStatus, 0);
  if (waitedPid < 0)
  {
    return NS_FAILURE;
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
  return NS_SUCCESS;
}

nsResult nsProcess::Launch(const nsProcessOptions& opt, nsBitflags<nsProcessLaunchFlags> launchFlags /*= nsProcessLaunchFlags::None*/)
{
  NS_ASSERT_DEV(m_pImpl->m_childPid == -1, "Can not reuse an instance of nsProcess");

  nsFd stdoutFd;
  nsFd stderrFd;

  if (nsProcessImpl::StartChildProcess(opt, m_pImpl->m_childPid, launchFlags.IsSet(nsProcessLaunchFlags::Suspended), stdoutFd, stderrFd).Failed())
  {
    return NS_FAILURE;
  }

  m_pImpl->m_exitCodeAvailable = false;
  m_pImpl->m_processSuspended = launchFlags.IsSet(nsProcessLaunchFlags::Suspended);

  if (stdoutFd.IsValid())
  {
    m_pImpl->AddStream(std::move(stdoutFd), opt.m_onStdOut);
  }

  if (stderrFd.IsValid())
  {
    m_pImpl->AddStream(std::move(stderrFd), opt.m_onStdError);
  }

  if (m_pImpl->GetNumStreams() > 0)
  {
    if (m_pImpl->StartStreamWatcher().Failed())
    {
      return NS_FAILURE;
    }
  }

  if (launchFlags.IsSet(nsProcessLaunchFlags::Detached))
  {
    Detach();
  }

  return NS_SUCCESS;
}

nsResult nsProcess::ResumeSuspended()
{
  if (m_pImpl->m_childPid < 0 || !m_pImpl->m_processSuspended)
  {
    return NS_FAILURE;
  }

  if (kill(m_pImpl->m_childPid, SIGCONT) < 0)
  {
    return NS_FAILURE;
  }
  m_pImpl->m_processSuspended = false;
  return NS_SUCCESS;
}

nsResult nsProcess::WaitToFinish(nsTime timeout /*= nsTime::MakeZero()*/)
{
  int childStatus = 0;
  NS_SCOPE_EXIT(m_pImpl->StopStreamWatcher());

  if (timeout.IsZero())
  {
    if (waitpid(m_pImpl->m_childPid, &childStatus, 0) < 0)
    {
      return NS_FAILURE;
    }
  }
  else
  {
    int waitResult = 0;
    nsTime startWait = nsTime::Now();
    while (true)
    {
      waitResult = waitpid(m_pImpl->m_childPid, &childStatus, WNOHANG);
      if (waitResult < 0)
      {
        return NS_FAILURE;
      }
      if (waitResult > 0)
      {
        break;
      }
      nsTime timeSpent = nsTime::Now() - startWait;
      if (timeSpent > timeout)
      {
        return NS_FAILURE;
      }
      nsThreadUtils::Sleep(nsMath::Min(nsTime::MakeFromMilliseconds(100.0), timeout - timeSpent));
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

  return NS_SUCCESS;
}

nsResult nsProcess::Terminate()
{
  if (m_pImpl->m_childPid == -1)
  {
    return NS_FAILURE;
  }

  NS_SCOPE_EXIT(m_pImpl->StopStreamWatcher());

  if (kill(m_pImpl->m_childPid, SIGKILL) < 0)
  {
    if (errno != ESRCH) // ESRCH = Process does not exist
    {
      return NS_FAILURE;
    }
  }
  m_pImpl->m_exitCodeAvailable = true;
  m_iExitCode = -1;

  return NS_SUCCESS;
}

nsProcessState nsProcess::GetState() const
{
  if (m_pImpl->m_childPid == -1)
  {
    return nsProcessState::NotStarted;
  }

  if (m_pImpl->m_exitCodeAvailable)
  {
    return nsProcessState::Finished;
  }

  int childStatus = -1;
  int waitResult = waitpid(m_pImpl->m_childPid, &childStatus, WNOHANG);
  if (waitResult > 0)
  {
    m_iExitCode = WEXITSTATUS(childStatus);
    m_pImpl->m_exitCodeAvailable = true;

    m_pImpl->StopStreamWatcher();

    return nsProcessState::Finished;
  }

  return nsProcessState::Running;
}

void nsProcess::Detach()
{
  m_pImpl->m_childPid = -1;
}

nsOsProcessHandle nsProcess::GetProcessHandle() const
{
  NS_ASSERT_DEV(false, "There is no process handle on posix");
  return nullptr;
}

nsOsProcessID nsProcess::GetProcessID() const
{
  NS_ASSERT_DEV(m_pImpl->m_childPid != -1, "No ProcessID available");
  return m_pImpl->m_childPid;
}

nsOsProcessID nsProcess::GetCurrentProcessID()
{
  return getpid();
}
