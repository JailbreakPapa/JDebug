#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/RemoteInterfaceEnet.h>

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT

#  include <Foundation/Logging/Log.h>
#  include <Foundation/Threading/ThreadUtils.h>
#  include <Foundation/Types/ScopeExit.h>
#  include <enet/enet.h>

class nsRemoteInterfaceEnetImpl : public nsRemoteInterfaceEnet
{

protected:
  virtual void InternalUpdateRemoteInterface() override;
  virtual nsResult InternalCreateConnection(nsRemoteMode mode, nsStringView sServerAddress) override;
  virtual void InternalShutdownConnection() override;
  virtual nsTime InternalGetPingToServer() override;
  virtual nsResult InternalTransmit(nsRemoteTransmitMode tm, const nsArrayPtr<const nsUInt8>& data) override;

private:
  ENetAddress m_EnetServerAddress;
  ENetHost* m_pEnetHost = nullptr;
  ENetPeer* m_pEnetConnectionToServer = nullptr;
  bool m_bAllowNetworkUpdates = true;
  nsMap<void*, nsUInt32> m_EnetPeerToClientID;

  static bool s_bEnetInitialized;
};

nsInternal::NewInstance<nsRemoteInterfaceEnet> nsRemoteInterfaceEnet::Make(nsAllocator* pAllocator /*= nsFoundation::GetDefaultAllocator()*/)
{
  return NS_NEW(pAllocator, nsRemoteInterfaceEnetImpl);
}

nsRemoteInterfaceEnet::nsRemoteInterfaceEnet() = default;
nsRemoteInterfaceEnet::~nsRemoteInterfaceEnet() = default;

bool nsRemoteInterfaceEnetImpl::s_bEnetInitialized = false;

nsResult nsRemoteInterfaceEnetImpl::InternalCreateConnection(nsRemoteMode mode, nsStringView sServerAddress)
{
  if (!s_bEnetInitialized)
  {
    if (enet_initialize() != 0)
    {
      nsLog::Error("Failed to initialize Enet");
      return NS_FAILURE;
    }

    s_bEnetInitialized = true;
  }

  {
    // Extract port from address
    const char* szPortStart = sServerAddress.FindLastSubString(":");
    nsStringView sPort = (szPortStart != nullptr) ? nsStringView(szPortStart + 1, sServerAddress.GetEndPointer()) : sServerAddress;
    nsInt32 iPort = 0;
    if (nsConversionUtils::StringToInt(sPort, iPort).Failed())
    {
      nsLog::Error("Failed to extract port from server address: {0}", sServerAddress);
      return NS_FAILURE;
    }
    m_uiPort = static_cast<nsUInt16>(iPort);
  }

  m_pEnetConnectionToServer = nullptr;

  ENetAddress* pServerAddress = nullptr;
  size_t maxPeerCount = 1;
  const size_t maxChannels = 2;
  const enet_uint32 incomingBandwidth = 0; // unlimited
  const enet_uint32 outgoingBandwidth = 0; // unlimited

  if (mode == nsRemoteMode::Server)
  {
    m_EnetServerAddress.host = ENET_HOST_ANY;
    m_EnetServerAddress.port = m_uiPort;

    maxPeerCount = 8;
    pServerAddress = &m_EnetServerAddress;
  }
  else
  {
    if (DetermineTargetAddress(sServerAddress, m_EnetServerAddress.host, m_EnetServerAddress.port).Failed())
    {
      nsStringBuilder tmp;
      enet_address_set_host(&m_EnetServerAddress, sServerAddress.GetData(tmp));
    }

    // use default settings for enet_host_create
  }

  m_pEnetHost = enet_host_create(pServerAddress, maxPeerCount, maxChannels, incomingBandwidth, outgoingBandwidth);

  if (m_pEnetHost == nullptr)
  {
    nsLog::Error("Failed to create an Enet server");
    return NS_FAILURE;
  }

  if (mode == nsRemoteMode::Client)
  {
    m_pEnetConnectionToServer = enet_host_connect(m_pEnetHost, &m_EnetServerAddress, maxChannels, GetConnectionToken());

    if (m_pEnetConnectionToServer == nullptr)
      return NS_FAILURE;
  }

  return NS_SUCCESS;
}

void nsRemoteInterfaceEnetImpl::InternalShutdownConnection()
{
  m_uiPort = 0;

  if (m_pEnetHost)
  {
    // send all peers that we are disconnecting
    for (nsUInt32 i = (nsUInt32)m_pEnetHost->connectedPeers; i > 0; --i)
      enet_peer_disconnect(&m_pEnetHost->peers[i - 1], 0);

    // process the network messages (e.g. send the disconnect messages)
    UpdateRemoteInterface();
    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
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

nsTime nsRemoteInterfaceEnetImpl::InternalGetPingToServer()
{
  NS_ASSERT_DEV(m_pEnetConnectionToServer != nullptr, "Client has not connected to server");

  enet_peer_ping(m_pEnetConnectionToServer);
  return nsTime::MakeFromMilliseconds(m_pEnetConnectionToServer->lastRoundTripTime);
}

nsResult nsRemoteInterfaceEnetImpl::InternalTransmit(nsRemoteTransmitMode tm, const nsArrayPtr<const nsUInt8>& data)
{
  if (m_pEnetHost == nullptr)
    return NS_FAILURE;

  ENetPacket* pPacket = enet_packet_create(data.GetPtr(), data.GetCount(), (tm == nsRemoteTransmitMode::Reliable) ? ENET_PACKET_FLAG_RELIABLE : 0);
  enet_host_broadcast(m_pEnetHost, 0, pPacket);

  return NS_SUCCESS;
}

void nsRemoteInterfaceEnetImpl::InternalUpdateRemoteInterface()
{
  if (!m_pEnetHost)
    return;

  if (!m_bAllowNetworkUpdates)
    return;

  m_bAllowNetworkUpdates = false;
  NS_SCOPE_EXIT(m_bAllowNetworkUpdates = true);

  ENetEvent NetworkEvent;

  while (true)
  {
    const nsInt32 iStatus = enet_host_service(m_pEnetHost, &NetworkEvent, 0);

    if (iStatus <= 0)
      return;

    switch (NetworkEvent.type)
    {
      case ENET_EVENT_TYPE_CONNECT:
      {
        if ((GetRemoteMode() == nsRemoteMode::Server) && (NetworkEvent.peer->eventData != GetConnectionToken()))
        {
          // do not accept connections that don't have the correct password
          enet_peer_disconnect(NetworkEvent.peer, 0);
          break;
        }

        // Uncomment this to allow stepping through enet code without loosing the connection.
        // enet_peer_timeout(NetworkEvent.peer, 0xFFFFFF, 32000, 0xFFFFFF);


        if (GetRemoteMode() == nsRemoteMode::Client)
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
          const nsUInt32 uiAppID = GetApplicationID();
          Send(nsRemoteTransmitMode::Reliable, GetConnectionToken(), 'NSID',
            nsArrayPtr<const nsUInt8>(reinterpret_cast<const nsUInt8*>(&uiAppID), sizeof(nsUInt32)));

          // then wait for its acknowledgment message
        }
      }
      break;

      case ENET_EVENT_TYPE_DISCONNECT:
      {
        if (GetRemoteMode() == nsRemoteMode::Client)
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
        const nsUInt32 uiApplicationID = *((nsUInt32*)&NetworkEvent.packet->data[0]);
        const nsUInt32 uiSystemID = *((nsUInt32*)&NetworkEvent.packet->data[4]);
        const nsUInt32 uiMsgID = *((nsUInt32*)&NetworkEvent.packet->data[8]);
        const nsUInt8* pData = &NetworkEvent.packet->data[12];

        if (uiSystemID == GetConnectionToken())
        {
          switch (uiMsgID)
          {
            case 'NSID':
            {
              // acknowledge that the ID has been received
              Send(GetConnectionToken(), 'AKID');

              // go tell the others about it
              nsUInt32 uiServerID = *((nsUInt32*)pData);
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
          ReportMessage(uiApplicationID, uiSystemID, uiMsgID, nsArrayPtr<const nsUInt8>(pData, (nsUInt32)NetworkEvent.packet->dataLength - 12));
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
