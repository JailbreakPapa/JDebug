#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Communication/IpcChannel.h>

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT

class wdRemoteInterface;
class wdRemoteMessage;

class WD_FOUNDATION_DLL wdIpcChannelEnet : public wdIpcChannel
{
public:
  wdIpcChannelEnet(const char* szAddress, Mode::Enum mode);
  ~wdIpcChannelEnet();

protected:
  virtual void InternalConnect() override;
  virtual void InternalDisconnect() override;
  virtual void InternalSend() override;
  virtual bool NeedWakeup() const override;
  virtual bool RequiresRegularTick() override { return true; }
  virtual void Tick() override;
  void NetworkMessageHandler(wdRemoteMessage& msg);
  void EnetEventHandler(const wdRemoteEvent& e);

  wdString m_sAddress;
  wdString m_sLastAddress;
  wdTime m_LastConnectAttempt;
  wdUniquePtr<wdRemoteInterface> m_pNetwork;
};

#endif
