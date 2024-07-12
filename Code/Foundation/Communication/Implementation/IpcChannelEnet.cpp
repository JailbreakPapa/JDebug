#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Implementation/IpcChannelEnet.h>

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT

#  include <Foundation/Communication/Implementation/MessageLoop.h>
#  include <Foundation/Communication/RemoteInterfaceEnet.h>
#  include <Foundation/Communication/RemoteMessage.h>
#  include <Foundation/Logging/Log.h>

nsIpcChannelEnet::nsIpcChannelEnet(nsStringView sAddress, Mode::Enum mode)
  : nsIpcChannel(sAddress, mode)
  , m_sAddress(sAddress)
{
  m_pNetwork = nsRemoteInterfaceEnet::Make();
  m_pNetwork->SetMessageHandler(0, nsMakeDelegate(&nsIpcChannelEnet::NetworkMessageHandler, this));
  m_pNetwork->m_RemoteEvents.AddEventHandler(nsMakeDelegate(&nsIpcChannelEnet::EnetEventHandler, this));

  m_pOwner->AddChannel(this);
}

nsIpcChannelEnet::~nsIpcChannelEnet()
{
  m_pNetwork->ShutdownConnection();

  m_pOwner->RemoveChannel(this);
}

void nsIpcChannelEnet::InternalConnect()
{
  if (m_Mode == Mode::Server)
  {
    m_pNetwork->StartServer('RMOT', m_sAddress, false).IgnoreResult();
    SetConnectionState(ConnectionState::Connecting);
  }
  else
  {
    SetConnectionState(ConnectionState::Connecting);
    if ((m_sLastAddress != m_sAddress) || (nsTime::Now() - m_LastConnectAttempt > nsTime::MakeFromSeconds(10)))
    {
      m_sLastAddress = m_sAddress;
      m_LastConnectAttempt = nsTime::Now();
      m_pNetwork->ConnectToServer('RMOT', m_sAddress, false).IgnoreResult();
    }

    m_pNetwork->WaitForConnectionToServer(nsTime::MakeFromMilliseconds(10.0)).IgnoreResult();
  }

  SetConnectionState(m_pNetwork->IsConnectedToOther() ? ConnectionState::Connected : ConnectionState::Disconnected);
}

void nsIpcChannelEnet::InternalDisconnect()
{
  m_pNetwork->ShutdownConnection();
  m_pNetwork->m_RemoteEvents.RemoveEventHandler(nsMakeDelegate(&nsIpcChannelEnet::EnetEventHandler, this));

  SetConnectionState(ConnectionState::Disconnected);
}

void nsIpcChannelEnet::InternalSend()
{
  {
    NS_LOCK(m_OutputQueueMutex);

    while (!m_OutputQueue.IsEmpty())
    {
      nsContiguousMemoryStreamStorage& storage = m_OutputQueue.PeekFront();

      m_pNetwork->Send(nsRemoteTransmitMode::Reliable, 0, 0, storage);

      m_OutputQueue.PopFront();
    }
  }

  m_pNetwork->UpdateRemoteInterface();
}

bool nsIpcChannelEnet::NeedWakeup() const
{
  return true;
}

void nsIpcChannelEnet::Tick()
{
  m_pNetwork->UpdateRemoteInterface();

  SetConnectionState(m_pNetwork->IsConnectedToOther() ? ConnectionState::Connected : ConnectionState::Disconnected);

  m_pNetwork->ExecuteAllMessageHandlers();
}

void nsIpcChannelEnet::NetworkMessageHandler(nsRemoteMessage& msg)
{
  ReceiveData(msg.GetMessageData());
}

void nsIpcChannelEnet::EnetEventHandler(const nsRemoteEvent& e)
{
  if (e.m_Type == nsRemoteEvent::DisconnectedFromServer)
  {
    nsLog::Info("Disconnected from remote engine process.");
    Disconnect();
  }

  if (e.m_Type == nsRemoteEvent::ConnectedToServer)
  {
    nsLog::Info("Connected to remote engine process.");
  }
}

#endif
