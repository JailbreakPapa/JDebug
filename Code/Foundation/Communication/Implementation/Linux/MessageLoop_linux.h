
#pragma once

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#if WD_ENABLED(WD_PLATFORM_LINUX)

#  include <Foundation/Basics.h>
#  include <Foundation/Communication/Implementation/MessageLoop.h>
#  include <Foundation/Threading/Mutex.h>

#  include <poll.h>

class wdIpcChannel;
class wdPipeChannel_linux;

WD_DEFINE_AS_POD_TYPE(struct pollfd);

class WD_FOUNDATION_DLL wdMessageLoop_linux : public wdMessageLoop
{
public:
  wdMessageLoop_linux();
  ~wdMessageLoop_linux();

protected:
  virtual void WakeUp() override;
  virtual bool WaitForMessages(wdInt32 iTimeout, wdIpcChannel* pFilter) override;

private:
  friend class wdPipeChannel_linux;

  enum class WaitType
  {
    Accept,
    IncomingMessage,
    Connect,
    Send
  };

  void RegisterWait(wdPipeChannel_linux* pChannel, WaitType type, int fd);
  void RemovePendingWaits(wdPipeChannel_linux* pChannel);

private:
  struct WaitInfo
  {
    WD_DECLARE_POD_TYPE();

    wdPipeChannel_linux* m_pChannel;
    WaitType m_type;
  };

  // m_waitInfos and m_pollInfos are alway the same size.
  // related information is stored at the same index.
  wdHybridArray<WaitInfo, 16> m_waitInfos;
  wdHybridArray<struct pollfd, 16> m_pollInfos;
  wdMutex m_pollMutex;
  wdAtomicInteger32 m_numPendingPollModifications = 0;
  int m_wakeupPipeReadEndFd = -1;
  int m_wakeupPipeWriteEndFd = -1;
};

#endif
