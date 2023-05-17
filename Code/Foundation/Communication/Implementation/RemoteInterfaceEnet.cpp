#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/RemoteInterfaceEnet.h>

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT

#  include <Foundation/Logging/Log.h>
#  include <Foundation/Threading/ThreadUtils.h>
#  include <Foundation/Types/ScopeExit.h>
#  include <enet/enet.h>

class wdRemoteInterfaceEnetImpl : public wdRemoteInterfaceEnet
{

protected:
  virtual void InternalUpdateRemoteInterface() override;
  virtual wdResult InternalCreateConnection(wdRemoteMode mode, const char* szServerAddress) override;
  virtual void InternalShutdownConnection() override;
  virtual wdTime InternalGetPingToServer() override;
  virtual wdResult InternalTransmit(wdRemoteTransmitMode tm, const wdArrayPtr<const wdUInt8>& data) override;

private:
  ENetAddress m_EnetServerAddress;
  ENetHost* m_pEnetHost = nullptr;
  ENetPeer* m_pEnetConnectionToServer = nullptr;
  bool m_bAllowNetworkUpdates = true;
  wdMap<void*, wdUInt32> m_EnetPeerToClientID;

  static bool s_bEnetInitialized;
};

wdInternal::NewInstance<wdRemoteInterfaceEnet> wdRemoteInterfaceEnet::Make(wdAllocatorBase* pAllocator /*= wdFoundation::GetDefaultAllocator()*/)
{
  return WD_NEW(pAllocator, wdRemoteInterfaceEnetImpl);
}

wdRemoteInterfaceEnet::wdRemoteInterfaceEnet() = default;
wdRemoteInterfaceEnet::~wdRemoteInterfaceEnet() = default;

bool wdRemoteInterfaceEnetImpl::s_bEnetInitialized = false;

wdResult wdRemoteInterfaceEnetImpl::InternalCreateConnection(wdRemoteMode mode, const char* szServerAddress)
{
  if (!s_bEnetInitialized)
  {
    if (enet_initialize() != 0)
    {
      wdLog::Error("Failed to initialize Enet");
      return WD_FAILURE;
    }

    s_bEnetInitialized = true;
  }

  {
    // Extract port from address
    const char* szPort = wdStringUtils::FindLastSubString(szServerAddress, ":");
    szPort = (szPort) ? szPort + 1 : szServerAddress;
    wdInt32 iPort = 0;
    if (wdConversionUtils::StringToInt(szPort, iPort).Failed())
    {
      wdLog::Error("Failed to extract port from server address: {0}", szServerAddress);
      return WD_FAILURE;
    }
    m_uiPort = static_cast<wdUInt16>(iPort);
  }

  m_pEnetConnectionToServer = nullptr;

  ENetAddress* pServerAddress = nullptr;
  size_t maxPeerCount = 1;
  const size_t maxChannels = 2;
  const enet_uint32 incomingBandwidth = 0; // unlimited
  const enet_uint32 outgoingBandwidth = 0; // unlimited

  if (mode == wdRemoteMode::Server)
  {
    m_EnetServerAddress.host = ENET_HOST_ANY;
    m_EnetServerAddress.port = m_uiPort;

    maxPeerCount = 8;
    pServerAddress = &m_EnetServerAddress;
  }
  else
  {
    if (DetermineTargetAddress(szServerAddress, m_EnetServerAddress.host, m_EnetServerAddress.port).Failed())
    {
      enet_address_set_host(&m_EnetServerAddress, szServerAddress);
    }

    // use default settings for enet_host_create
  }

  m_pEnetHost = enet_host_create(pServerAddress, maxPeerCount, maxChannels, incomingBandwidth, outgoingBandwidth);

  if (m_pEnetHost == nullptr)
  {
    wdLog::Error("Failed to create an Enet server");
    return WD_FAILURE;
  }

  if (mode == wdRemoteMode::Client)
  {
    m_pEnetConnectionToServer = enet_host_connect(m_pEnetHost, &m_EnetServerAddress, maxChannels, GetConnectionToken());

    if (m_pEnetConnectionToServer == nullptr)
      return WD_FAILURE;
  }

  return WD_SUCCESS;
}

void wdRemoteInterfaceEnetImpl::InternalShutdownConnection()
{
  m_uiPort = 0;

  if (m_pEnetHost)
  {
    // send all peers that we are disconnecting
    for (wdUInt32 i = (wdUInt32)m_pEnetHost->connectedPeers; i > 0; --i)
      enet_peer_disconnect(&m_pEnetHost->peers[i - 1], 0);

    // process the network messages (e.g. send the disconnect messages)
    UpdateRemoteInterface();
    wdThreadUtils::Sleep(wdTime::Milliseconds(10));
  }

  // finally close the network connection
  if (m_pEnetHost)
  {
    enet_host_destroy(m_pEnetHost);
    m_pEnetHost = nullptr;
  }

  // enet_deinitialize();
  m_pEnetConnectionToServer = nullptr;
}

wdTime wdRemoteInterfaceEnetImpl::InternalGetPingToServer()
{
  WD_ASSERT_DEV(m_pEnetConnectionToServer != nullptr, "Client has not connected to server");

  enet_peer_ping(m_pEnetConnectionToServer);
  return wdTime::Milliseconds(m_pEnetConnectionToServer->lastRoundTripTime);
}

wdResult wdRemoteInterfaceEnetImpl::InternalTransmit(wdRemoteTransmitMode tm, const wdArrayPtr<const wdUInt8>& data)
{
  if (m_pEnetHost == nullptr)
    return WD_FAILURE;

  ENetPacket* pPacket = enet_packet_create(data.GetPtr(), data.GetCount(), (tm == wdRemoteTransmitMode::Reliable) ? ENET_PACKET_FLAG_RELIABLE : 0);
  enet_host_broadcast(m_pEnetHost, 0, pPacket);

  return WD_SUCCESS;
}

void wdRemoteInterfaceEnetImpl::InternalUpdateRemoteInterface()
{
  if (!m_pEnetHost)
    return;

  if (!m_bAllowNetworkUpdates)
    return;

  m_bAllowNetworkUpdates = false;
  WD_SCOPE_EXIT(m_bAllowNetworkUpdates = true);

  ENetEvent NetworkEvent;

  while (true)
  {
    const wdInt32 iStatus = enet_host_service(m_pEnetHost, &NetworkEvent, 0);

    if (iStatus <= 0)
      return;

    switch (NetworkEvent.type)
    {
      case ENET_EVENT_TYPE_CONNECT:
      {
        if ((GetRemoteMode() == wdRemoteMode::Server) && (NetworkEvent.peer->eventData != GetConnectionToken()))
        {
          // do not accept connections that don't have the correct password
          enet_peer_disconnect(NetworkEvent.peer, 0);
          break;
        }

        // Uncomment this to allow stepping through enet code without loosing the connection.
        // enet_peer_timeout(NetworkEvent.peer, 0xFFFFFF, 32000, 0xFFFFFF);


        if (GetRemoteMode() == wdRemoteMode::Client)
        {
          // Querying host IP and name can take a lot of time
          // Do not do this in the other case, as it may result in timeouts while establishing the connection.
          char szHostIP[64] = "<unknown>";
          // char szHostName[64] = "<unknown>";

          enet_address_get_host_ip(&NetworkEvent.peer->address, szHostIP, 63);
          // enet_address_get_host(&NetworkEvent.peer->address, szHostName, 63);

          m_sServerInfoIP = szHostIP;
          // m_ServerInfoName = szHostName;

          // now we are waiting for the server to send its ID
        }
        else
        {
          const wdUInt32 uiAppID = GetApplicationID();
          Send(wdRemoteTransmitMode::Reliable, GetConnectionToken(), 'WDID',
            wdArrayPtr<const wdUInt8>(reinterpret_cast<const wdUInt8*>(&uiAppID), sizeof(wdUInt32)));

          // then wait for its acknowledgment message
        }
      }
      break;

      case ENET_EVENT_TYPE_DISCONNECT:
      {
        if (GetRemoteMode() == wdRemoteMode::Client)
        {
          ReportDisconnectedFromServer();
        }
        else
        {
          auto it = m_EnetPeerToClientID.Find(NetworkEvent.peer);
          if (it.IsValid())
          {
            ReportDisconnectedFromClient(it.Value());
            m_EnetPeerToClientID.Remove(it);
          }
        }
      }
      break;

      case ENET_EVENT_TYPE_RECEIVE:
      {
        const wdUInt32 uiApplicationID = *((wdUInt32*)&NetworkEvent.packet->data[0]);
        const wdUInt32 uiSystemID = *((wdUInt32*)&NetworkEvent.packet->data[4]);
        const wdUInt32 uiMsgID = *((wdUInt32*)&NetworkEvent.packet->data[8]);
        const wdUInt8* pData = &NetworkEvent.packet->data[12];

        if (uiSystemID == GetConnectionToken())
        {
          switch (uiMsgID)
          {
            case 'WDID':
            {
              // acknowledge that the ID has been received
              Send(GetConnectionToken(), 'AKID');

              // go tell the others about it
              wdUInt32 uiServerID = *((wdUInt32*)pData);
              ReportConnectionToServer(uiServerID);
            }
            break;

            case 'AKID':
            {
              if (m_EnetPeerToClientID[NetworkEvent.peer] != uiApplicationID)
              {
                m_EnetPeerToClientID[NetworkEvent.peer] = uiApplicationID;

                // the client received the server ID -> the connection has been established properly
                ReportConnectionToClient(uiApplicationID);
              }
            }
            break;
          }
        }
        else
        {
          ReportMessage(uiApplicationID, uiSystemID, uiMsgID, wdArrayPtr<const wdUInt8>(pData, (wdUInt32)NetworkEvent.packet->dataLength - 12));
        }

        enet_packet_destroy(NetworkEvent.packet);
      }
      break;

      default:
        break;
    }
  }
}

#endif



WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_RemoteInterfaceEnet);
