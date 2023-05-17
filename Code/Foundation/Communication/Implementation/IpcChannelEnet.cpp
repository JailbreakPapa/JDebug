#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Implementation/IpcChannelEnet.h>

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT

#  include <Foundation/Communication/Implementation/MessageLoop.h>
#  include <Foundation/Communication/RemoteInterfaceEnet.h>
#  include <Foundation/Communication/RemoteMessage.h>
#  include <Foundation/Logging/Log.h>

wdIpcChannelEnet::wdIpcChannelEnet(const char* szAddress, Mode::Enum mode)
  : wdIpcChannel(szAddress, mode)
{
  m_sAddress = szAddress;
  m_pNetwork = wdRemoteInterfaceEnet::Make();
  m_pNetwork->SetMessageHandler(0, wdMakeDelegate(&wdIpcChannelEnet::NetworkMessageHandler, this));
  m_pNetwork->m_RemoteEvents.AddEventHandler(wdMakeDelegate(&wdIpcChannelEnet::EnetEventHandler, this));

  m_pOwner->AddChannel(this);
}

wdIpcChannelEnet::~wdIpcChannelEnet()
{
  m_pNetwork->ShutdownConnection();

  m_pOwner->RemoveChannel(this);
}

void wdIpcChannelEnet::InternalConnect()
{
  if (m_Mode == Mode::Server)
  {
    m_pNetwork->StartServer('RMOT', m_sAddress, false).IgnoreResult();
  }
  else
  {
    if ((m_sLastAddress != m_sAddress) || (wdTime::Now() - m_LastConnectAttempt > wdTime::Seconds(10)))
    {
      m_sLastAddress = m_sAddress;
      m_LastConnectAttempt = wdTime::Now();
      m_pNetwork->ConnectToServer('RMOT', m_sAddress, false).IgnoreResult();
    }

    m_pNetwork->WaitForConnectionToServer(wdTime::Milliseconds(10.0)).IgnoreResult();
  }

  m_bConnected = m_pNetwork->IsConnectedToOther() ? 1 : 0;
}

void wdIpcChannelEnet::InternalDisconnect()
{
  m_pNetwork->ShutdownConnection();
  m_pNetwork->m_RemoteEvents.RemoveEventHandler(wdMakeDelegate(&wdIpcChannelEnet::EnetEventHandler, this));

  m_bConnected = 0;
}

void wdIpcChannelEnet::InternalSend()
{
  {
    WD_LOCK(m_OutputQueueMutex);

    while (!m_OutputQueue.IsEmpty())
    {
      wdContiguousMemoryStreamStorage& storage = m_OutputQueue.PeekFront();

      m_pNetwork->Send(wdRemoteTransmitMode::Reliable, 0, 0, storage);

      m_OutputQueue.PopFront();
    }
  }

  m_pNetwork->UpdateRemoteInterface();
}

bool wdIpcChannelEnet::NeedWakeup() const
{
  return true;
}

void wdIpcChannelEnet::Tick()
{
  m_pNetwork->UpdateRemoteInterface();

  m_bConnected = m_pNetwork->IsConnectedToOther() ? 1 : 0;

  m_pNetwork->ExecuteAllMessageHandlers();
}

void wdIpcChannelEnet::NetworkMessageHandler(wdRemoteMessage& msg)
{
  ReceiveMessageData(msg.GetMessageData());
}

void wdIpcChannelEnet::EnetEventHandler(const wdRemoteEvent& e)
{
  if (e.m_Type == wdRemoteEvent::DisconnectedFromServer)
  {
    wdLog::Info("Disconnected from remote engine process.");
    Disconnect();
  }

  if (e.m_Type == wdRemoteEvent::ConnectedToServer)
  {
    wdLog::Info("Connected to remote engine process.");
  }
}

#endif



WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_IpcChannelEnet);
