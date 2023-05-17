#pragma once

#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#if WD_ENABLED(WD_PLATFORM_LINUX)

#  include <Foundation/Basics.h>
#  include <Foundation/Communication/IpcChannel.h>

#  include <sys/stat.h>
#  include <sys/types.h>


class WD_FOUNDATION_DLL wdPipeChannel_linux : public wdIpcChannel
{
public:
  wdPipeChannel_linux(const char* szAddress, Mode::Enum mode);
  ~wdPipeChannel_linux();

private:
  friend class wdMessageLoop;
  friend class wdMessageLoop_linux;

  virtual void AddToMessageLoop(wdMessageLoop* pMsgLoop) override;

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
  wdString m_serverSocketPath;
  wdString m_clientSocketPath;
  int m_serverSocketFd = -1;
  int m_clientSocketFd = -1;

  wdUInt8 m_InputBuffer[4096];
  wdAtomicBool m_Connecting = false;

  wdUInt64 m_previousSendOffset = 0;
};
#endif