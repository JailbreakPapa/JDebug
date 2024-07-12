#pragma once

#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#if NS_ENABLED(NS_PLATFORM_LINUX)

#  include <Foundation/Basics.h>
#  include <Foundation/Communication/IpcChannel.h>

#  include <sys/stat.h>
#  include <sys/types.h>


class NS_FOUNDATION_DLL nsPipeChannel_linux : public nsIpcChannel
{
public:
  nsPipeChannel_linux(nsStringView sAddress, Mode::Enum mode);
  ~nsPipeChannel_linux();

private:
  friend class nsMessageLoop;
  friend class nsMessageLoop_linux;

  // All functions from here on down are run from worker thread only
  virtual void InternalConnect() override;
  virtual void InternalDisconnect() override;
  virtual void InternalSend() override;
  virtual bool NeedWakeup() const override;

  // These are called from MessageLoop_linux on OS events
  void AcceptIncomingConnection();
  void ProcessIncomingPackages();
  void ProcessConnectSuccessfull();

private:
  nsString m_serverSocketPath;
  nsString m_clientSocketPath;
  int m_serverSocketFd = -1;
  int m_clientSocketFd = -1;

  nsUInt8 m_InputBuffer[4096];
  nsUInt64 m_previousSendOffset = 0;
};
#endif
