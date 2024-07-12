
#pragma once

#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#if NS_ENABLED(NS_PLATFORM_LINUX)

#  include <Foundation/Basics.h>
#  include <Foundation/Communication/Implementation/MessageLoop.h>
#  include <Foundation/Threading/Mutex.h>

#  include <poll.h>

class nsIpcChannel;
class nsPipeChannel_linux;

#  ifndef _NS_DEFINED_POLLFD_POD
#    define _NS_DEFINED_POLLFD_POD
NS_DEFINE_AS_POD_TYPE(struct pollfd);
#  endif

class NS_FOUNDATION_DLL nsMessageLoop_linux : public nsMessageLoop
{
public:
  nsMessageLoop_linux();
  ~nsMessageLoop_linux();

protected:
  virtual void WakeUp() override;
  virtual bool WaitForMessages(nsInt32 iTimeout, nsIpcChannel* pFilter) override;

private:
  friend class nsPipeChannel_linux;

  enum class WaitType
  {
    Accept,
    IncomingMessage,
    Connect,
    Send
  };

  void RegisterWait(nsPipeChannel_linux* pChannel, WaitType type, int fd);
  void RemovePendingWaits(nsPipeChannel_linux* pChannel);

private:
  struct WaitInfo
  {
    NS_DECLARE_POD_TYPE();

    nsPipeChannel_linux* m_pChannel;
    WaitType m_type;
  };

  // m_waitInfos and m_pollInfos are alway the same size.
  // related information is stored at the same index.
  nsHybridArray<WaitInfo, 16> m_waitInfos;
  nsHybridArray<struct pollfd, 16> m_pollInfos;
  nsMutex m_pollMutex;
  nsAtomicInteger32 m_numPendingPollModifications = 0;
  int m_wakeupPipeReadEndFd = -1;
  int m_wakeupPipeWriteEndFd = -1;
};

#endif
