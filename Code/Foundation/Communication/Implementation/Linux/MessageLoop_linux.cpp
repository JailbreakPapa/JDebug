#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_LINUX)

#  include <Foundation/Communication/Implementation/Linux/MessageLoop_linux.h>

#  include <Foundation/Communication/Implementation/Linux/PipeChannel_linux.h>
#  include <Foundation/Logging/Log.h>

#  include <fcntl.h>
#  include <poll.h>
#  include <unistd.h>

wdMessageLoop_linux::wdMessageLoop_linux()
{
  int fds[2];
  if (pipe2(fds, O_NONBLOCK | O_CLOEXEC) < 0)
  {
    wdLog::Error("[IPC]Failed to create wakeup pipe for wdMessageLoop_linux");
  }
  else
  {
    m_wakeupPipeReadEndFd = fds[0];
    m_wakeupPipeWriteEndFd = fds[1];

    // Setup always present wait info for magic index 0 = wake up
    m_pollInfos.PushBack({m_wakeupPipeReadEndFd, POLLIN, 0});
    m_waitInfos.PushBack({nullptr, WaitType::Accept});
  }
}

wdMessageLoop_linux::~wdMessageLoop_linux()
{
  StopUpdateThread();
  if (m_wakeupPipeReadEndFd >= 0)
  {
    close(m_wakeupPipeReadEndFd);
    close(m_wakeupPipeWriteEndFd);
  }
}

bool wdMessageLoop_linux::WaitForMessages(wdInt32 iTimeout, wdIpcChannel* pFilter)
{
  WD_ASSERT_ALWAYS(pFilter == nullptr, "Not implemented");

  while (m_numPendingPollModifications > 0)
  {
    wdThreadUtils::YieldTimeSlice();
  }

  wdLock lock{m_pollMutex};
  int result = poll(m_pollInfos.GetData(), m_pollInfos.GetCount(), iTimeout);
  if (result > 0)
  {
    // Result at index 0 is special and means there was a WakeUp
    if (m_pollInfos[0].revents != 0)
    {
      wdUInt8 wakeupByte;
      auto readResult = read(m_wakeupPipeReadEndFd, &wakeupByte, sizeof(wakeupByte));
      m_pollInfos[0].revents = 0;
      return true;
    }

    wdUInt32 numEvents = m_pollInfos.GetCount();
    for (wdUInt32 i = 1; i < numEvents;)
    {
      WaitInfo& waitInfo = m_waitInfos[i];
      struct pollfd& pollInfo = m_pollInfos[i];
      if (pollInfo.revents != 0)
      {
        switch (waitInfo.m_type)
        {
          case WaitType::Accept:
            waitInfo.m_pChannel->AcceptIncomingConnection();
            m_pollInfos.RemoveAtAndSwap(i);
            m_waitInfos.RemoveAtAndSwap(i);
            numEvents--;
            continue;
          case WaitType::Connect:
            waitInfo.m_pChannel->ProcessConnectSuccessfull();
            m_pollInfos.RemoveAtAndSwap(i);
            m_waitInfos.RemoveAtAndSwap(i);
            numEvents--;
            continue;
          case WaitType::IncomingMessage:
            waitInfo.m_pChannel->ProcessIncomingPackages();
            break;
          case WaitType::Send:
            waitInfo.m_pChannel->InternalSend();
            m_pollInfos.RemoveAtAndSwap(i);
            m_waitInfos.RemoveAtAndSwap(i);
            numEvents--;
            continue;
        }
        pollInfo.revents = 0;
      }
      ++i;
    }
    return true;
  }
  if (result < 0)
  {
    wdLog::Error("[IPC]wdMessageLoop_linux failed to poll events. Error {}", errno);
  }
  return false;
}

void wdMessageLoop_linux::RegisterWait(wdPipeChannel_linux* pChannel, WaitType type, int fd)
{
  wdLog::Debug("[IPC]wdMessageLoop_linux::RegisterWait({}}", (int)type);
  short int waitFlags = 0;
  switch (type)
  {
    case WaitType::Accept:
      waitFlags = POLLIN;
      break;
    case WaitType::Connect:
      waitFlags = POLLOUT;
      break;
    case WaitType::IncomingMessage:
      waitFlags = POLLIN;
      break;
    case WaitType::Send:
      waitFlags = POLLOUT;
      break;
  }

  m_numPendingPollModifications.Increment();
  WD_SCOPE_EXIT(m_numPendingPollModifications.Decrement());
  WakeUp();
  {
    wdLock lock{m_pollMutex};
    m_waitInfos.PushBack({pChannel, type});
    m_pollInfos.PushBack({fd, waitFlags, 0});
  }
}

void wdMessageLoop_linux::RemovePendingWaits(wdPipeChannel_linux* pChannel)
{
  m_numPendingPollModifications.Increment();
  WD_SCOPE_EXIT(m_numPendingPollModifications.Decrement());
  WakeUp();
  {
    wdLock lock{m_pollMutex};
    wdUInt32 waitCount = m_pollInfos.GetCount();
    for (wdUInt32 i = 0; i < waitCount;)
    {
      if (m_waitInfos[i].m_pChannel == pChannel)
      {
        m_waitInfos.RemoveAtAndSwap(i);
        m_pollInfos.RemoveAtAndSwap(i);
        waitCount--;
      }
      else
      {
        i++;
      }
    }
  }
}

void wdMessageLoop_linux::WakeUp()
{
  wdUInt8 wakeupByte = 0;
  int writeResult = write(m_wakeupPipeWriteEndFd, &wakeupByte, sizeof(wakeupByte));
}

#endif


WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_Linux_MessageLoop_linux);
