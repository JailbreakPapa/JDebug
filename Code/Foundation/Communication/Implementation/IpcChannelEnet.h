#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/IpcChannel.h>

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT

class nsRemoteInterface;
class nsRemoteMessage;

class NS_FOUNDATION_DLL nsIpcChannelEnet : public nsIpcChannel
{
public:
  nsIpcChannelEnet(nsStringView sAddress, Mode::Enum mode);
  ~nsIpcChannelEnet();

protected:
  virtual void InternalConnect() override;
  virtual void InternalDisconnect() override;
  virtual void InternalSend() override;
  virtual bool NeedWakeup() const override;
  virtual bool RequiresRegularTick() override { return true; }
  virtual void Tick() override;
  void NetworkMessageHandler(nsRemoteMessage& msg);
  void EnetEventHandler(const nsRemoteEvent& e);

  nsString m_sAddress;
  nsString m_sLastAddress;
  nsTime m_LastConnectAttempt;
  nsUniquePtr<nsRemoteInterface> m_pNetwork;
};

#endif
