#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Threading/ThreadUtils.h>

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
#  include <enet/enet.h>
#endif

class wdTelemetryThread;

wdTelemetry::wdEventTelemetry wdTelemetry::s_TelemetryEvents;
wdUInt32 wdTelemetry::s_uiApplicationID = 0;
wdUInt32 wdTelemetry::s_uiServerID = 0;
wdUInt16 wdTelemetry::s_uiPort = 1040;
bool wdTelemetry::s_bConnectedToServer = false;
bool wdTelemetry::s_bConnectedToClient = false;
bool wdTelemetry::s_bAllowNetworkUpdate = true;
wdTime wdTelemetry::s_PingToServer;
wdString wdTelemetry::s_sServerName;
wdString wdTelemetry::s_sServerIP;
static bool g_bInitialized = false;
wdTelemetry::ConnectionMode wdTelemetry::s_ConnectionMode = wdTelemetry::None;
wdMap<wdUInt64, wdTelemetry::MessageQueue> wdTelemetry::s_SystemMessages;

#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
static ENetAddress g_pServerAddress;
static ENetHost* g_pHost = nullptr;
static ENetPeer* g_pConnectionToServer = nullptr;
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT

void wdTelemetry::UpdateServerPing()
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  enet_peer_ping(g_pConnectionToServer);
  wdTelemetry::s_PingToServer = wdTime::Milliseconds(g_pConnectionToServer->lastRoundTripTime);
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void wdTelemetry::UpdateNetwork()
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  if (!g_pHost)
    return;

  if (!s_bAllowNetworkUpdate)
    return;

  s_bAllowNetworkUpdate = false;

  ENetEvent NetworkEvent;

  while (true)
  {
    WD_LOCK(GetTelemetryMutex());

    const wdInt32 iStatus = enet_host_service(g_pHost, &NetworkEvent, 0);

    if (iStatus <= 0)
    {
      s_bAllowNetworkUpdate = true;
      return;
    }

    switch (NetworkEvent.type)
    {
      case ENET_EVENT_TYPE_CONNECT:
      {
        if ((wdTelemetry::s_ConnectionMode == wdTelemetry::Server) && (NetworkEvent.peer->eventData != 'WDBC'))
        {
          enet_peer_disconnect(NetworkEvent.peer, 0);
          break;
        }

        if (s_ConnectionMode == Client)
        {
          char szHostIP[64] = "<unknown>";
          // char szHostName[64] = "<unknown>";

          enet_address_get_host_ip(&NetworkEvent.peer->address, szHostIP, 63);

          // Querying host IP and name can take a lot of time which can lead to timeouts
          // enet_address_get_host(&NetworkEvent.peer->address, szHostName, 63);

          wdTelemetry::s_sServerIP = szHostIP;
          // wdTelemetry::s_ServerName = szHostName;

          // now we are waiting for the server to send its ID
        }
        else
        {
          // got a new client, send the server ID to it
          s_bConnectedToClient = true; // we need this fake state, otherwise Broadcast will queue the message instead of sending it
          Broadcast(wdTelemetry::Reliable, 'WDBC', 'WDID', &s_uiApplicationID, sizeof(wdUInt32));
          s_bConnectedToClient = false;

          // then wait for its acknowledgment message
        }
      }
      break;

      case ENET_EVENT_TYPE_DISCONNECT:
      {
        if (s_ConnectionMode == Client)
        {
          s_bConnectedToServer = false;

          // First wait a bit to ensure that the Server could shut down, if this was a legitimate disconnect
          wdThreadUtils::Sleep(wdTime::Seconds(1));

          // Now try to reconnect. If the Server still exists, fine, connect to that.
          // If it does not exist anymore, this will connect to the next best Server that can be found.
          g_pConnectionToServer = enet_host_connect(g_pHost, &g_pServerAddress, 2, 'WDBC');

          TelemetryEventData e;
          e.m_EventType = TelemetryEventData::DisconnectedFromServer;

          s_TelemetryEvents.Broadcast(e);
        }
        else
        {
          /// \todo This assumes we only connect to a single client ...
          s_bConnectedToClient = false;

          TelemetryEventData e;
          e.m_EventType = TelemetryEventData::DisconnectedFromClient;

          s_TelemetryEvents.Broadcast(e);
        }
      }
      break;

      case ENET_EVENT_TYPE_RECEIVE:
      {
        const wdUInt32 uiSystemID = *((wdUInt32*)&NetworkEvent.packet->data[0]);
        const wdUInt32 uiMsgID = *((wdUInt32*)&NetworkEvent.packet->data[4]);
        const wdUInt8* pData = &NetworkEvent.packet->data[8];

        if (uiSystemID == 'WDBC')
        {
          switch (uiMsgID)
          {
            case 'WDID':
            {
              s_uiServerID = *((wdUInt32*)pData);

              // connection to server is finalized
              s_bConnectedToServer = true;

              // acknowledge that the ID has been received
              SendToServer('WDBC', 'AKID', nullptr, 0);

              // go tell the others about it
              TelemetryEventData e;
              e.m_EventType = TelemetryEventData::ConnectedToServer;

              s_TelemetryEvents.Broadcast(e);

              FlushOutgoingQueues();
            }
            break;
            case 'AKID':
            {
              // the client received the server ID -> the connection has been established properly

              /// \todo This assumes we only connect to a single client ...
              s_bConnectedToClient = true;

              // go tell the others about it
              TelemetryEventData e;
              e.m_EventType = TelemetryEventData::ConnectedToClient;

              s_TelemetryEvents.Broadcast(e);

              SendServerName();
              FlushOutgoingQueues();
            }
            break;

            case 'NAME':
            {
              s_sServerName = reinterpret_cast<const char*>(pData);
            }
            break;
          }
        }
        else
        {
          MessageQueue& Queue = s_SystemMessages[uiSystemID];

          if (Queue.m_bAcceptMessages)
          {
            Queue.m_IncomingQueue.PushBack();
            wdTelemetryMessage& Msg = Queue.m_IncomingQueue.PeekBack();

            Msg.SetMessageID(uiSystemID, uiMsgID);

            WD_ASSERT_DEV((wdUInt32)NetworkEvent.packet->dataLength >= 8, "Message Length Invalid: {0}", (wdUInt32)NetworkEvent.packet->dataLength);

            Msg.GetWriter().WriteBytes(pData, NetworkEvent.packet->dataLength - 8).IgnoreResult();
          }
        }

        enet_packet_destroy(NetworkEvent.packet);
      }
      break;

      default:
        break;
    }
  }

  s_bAllowNetworkUpdate = true;
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void wdTelemetry::SetServerName(const char* szName)
{
  if (s_ConnectionMode == ConnectionMode::Client)
    return;

  if (s_sServerName == szName)
    return;

  s_sServerName = szName;

  SendServerName();
}

void wdTelemetry::SendServerName()
{
  if (!IsConnectedToOther())
    return;

  char data[48];
  wdStringUtils::Copy(data, WD_ARRAY_SIZE(data), s_sServerName.GetData());

  Broadcast(wdTelemetry::Reliable, 'WDBC', 'NAME', data, WD_ARRAY_SIZE(data));
}

wdResult wdTelemetry::RetrieveMessage(wdUInt32 uiSystemID, wdTelemetryMessage& out_message)
{
  if (s_SystemMessages[uiSystemID].m_IncomingQueue.IsEmpty())
    return WD_FAILURE;

  WD_LOCK(GetTelemetryMutex());

  // check again while inside the lock
  if (s_SystemMessages[uiSystemID].m_IncomingQueue.IsEmpty())
    return WD_FAILURE;

  out_message = s_SystemMessages[uiSystemID].m_IncomingQueue.PeekFront();
  s_SystemMessages[uiSystemID].m_IncomingQueue.PopFront();

  return WD_SUCCESS;
}

void wdTelemetry::InitializeAsServer()
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  g_pServerAddress.host = ENET_HOST_ANY;
  g_pServerAddress.port = s_uiPort;

  g_pHost = enet_host_create(&g_pServerAddress, 32, 2, 0, 0);
#else
  wdLog::SeriousWarning("Enet is not compiled into this build, wdTelemetry::InitializeAsServer() will be ignored.");
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

wdResult wdTelemetry::InitializeAsClient(const char* szConnectTo)
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  g_pHost = enet_host_create(nullptr, 1, 2, 0, 0);

  wdStringBuilder sConnectTo = szConnectTo;

  const char* szColon = sConnectTo.FindLastSubString(":");
  if (szColon != nullptr)
  {
    sConnectTo.Shrink(0, wdStringUtils::GetStringElementCount(szColon));

    wdStringBuilder sPort = szColon + 1;
    s_uiPort = static_cast<wdUInt16>(atoi(sPort.GetData()));
  }

  if (sConnectTo.IsEmpty() || sConnectTo.IsEqual_NoCase("localhost"))
    enet_address_set_host(&g_pServerAddress, "localhost");
  else if (sConnectTo.FindSubString(".") != nullptr)
  {
    wdHybridArray<wdString, 8> IP;
    sConnectTo.Split(false, IP, ".");

    if (IP.GetCount() != 4)
      return WD_FAILURE;

    const wdUInt32 ip1 = atoi(IP[0].GetData()) & 0xFF;
    const wdUInt32 ip2 = atoi(IP[1].GetData()) & 0xFF;
    const wdUInt32 ip3 = atoi(IP[2].GetData()) & 0xFF;
    const wdUInt32 ip4 = atoi(IP[3].GetData()) & 0xFF;

    const wdUInt32 uiIP = (ip1 | ip2 << 8 | ip3 << 16 | ip4 << 24);

    g_pServerAddress.host = uiIP;
  }
  else
    enet_address_set_host(&g_pServerAddress, sConnectTo.GetData());

  g_pServerAddress.port = s_uiPort;

  g_pConnectionToServer = nullptr;
  g_pConnectionToServer = enet_host_connect(g_pHost, &g_pServerAddress, 2, 'WDBC');

  if (g_pConnectionToServer)
    return WD_SUCCESS;
#else
  wdLog::SeriousWarning("Enet is not compiled into this build, wdTelemetry::InitializeAsClient() will be ignored.");
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT

  return WD_FAILURE;
}

wdResult wdTelemetry::OpenConnection(ConnectionMode Mode, const char* szConnectTo)
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  CloseConnection();

  if (!g_bInitialized)
  {
    if (enet_initialize() != 0)
    {
      wdLog::Error("Enet could not be initialized.");
      return WD_FAILURE;
    }

    g_bInitialized = true;
  }

  s_uiApplicationID = (wdUInt32)wdTime::Now().GetSeconds();

  switch (Mode)
  {
    case wdTelemetry::Server:
      InitializeAsServer();
      break;
    case wdTelemetry::Client:
      if (InitializeAsClient(szConnectTo) == WD_FAILURE)
      {
        CloseConnection();
        return WD_FAILURE;
      }
      break;
    default:
      break;
  }

  s_ConnectionMode = Mode;

  wdTelemetry::UpdateNetwork();

  StartTelemetryThread();

  return WD_SUCCESS;
#else
  wdLog::SeriousWarning("Enet is not compiled into this build, wdTelemetry::OpenConnection() will be ignored.");
  return WD_FAILURE;
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void wdTelemetry::Transmit(TransmitMode tm, const void* pData, wdUInt32 uiDataBytes)
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  if (!g_pHost)
    return;

  WD_LOCK(GetTelemetryMutex());

  ENetPacket* pPacket = enet_packet_create(pData, uiDataBytes, (tm == Reliable) ? ENET_PACKET_FLAG_RELIABLE : 0);
  enet_host_broadcast(g_pHost, 0, pPacket);

  // make sure the message is processed immediately
  wdTelemetry::UpdateNetwork();
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void wdTelemetry::Send(TransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const void* pData, wdUInt32 uiDataBytes)
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  if (!g_pHost)
    return;

  // in case we have no connection to a peer, queue the message
  if (!IsConnectedToOther())
    QueueOutgoingMessage(tm, uiSystemID, uiMsgID, pData, uiDataBytes);
  else
  {
    // when we do have a connection, just send the message out

    wdHybridArray<wdUInt8, 64> TempData;
    TempData.SetCountUninitialized(8 + uiDataBytes);
    *((wdUInt32*)&TempData[0]) = uiSystemID;
    *((wdUInt32*)&TempData[4]) = uiMsgID;

    if (pData && uiDataBytes > 0)
      wdMemoryUtils::Copy((wdUInt8*)&TempData[8], (wdUInt8*)pData, uiDataBytes);

    Transmit(tm, &TempData[0], TempData.GetCount());
  }
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void wdTelemetry::Send(TransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, wdStreamReader& Stream, wdInt32 iDataBytes)
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  if (!g_pHost)
    return;

  const wdUInt32 uiStackSize = 1024;

  wdHybridArray<wdUInt8, uiStackSize + 8> TempData;
  TempData.SetCountUninitialized(8);
  *((wdUInt32*)&TempData[0]) = uiSystemID;
  *((wdUInt32*)&TempData[4]) = uiMsgID;

  // if we don't know how much to take out of the stream, read the data piece by piece from the input stream
  if (iDataBytes < 0)
  {
    while (true)
    {
      const wdUInt32 uiOffset = TempData.GetCount();
      TempData.SetCountUninitialized(uiOffset + uiStackSize); // no allocation the first time

      const wdUInt32 uiRead = static_cast<wdUInt32>(Stream.ReadBytes(&TempData[uiOffset], uiStackSize));

      if (uiRead < uiStackSize)
      {
        // resize the array down to its actual size
        TempData.SetCountUninitialized(uiOffset + uiRead);
        break;
      }
    }
  }
  else
  {
    TempData.SetCountUninitialized(8 + iDataBytes);

    if (iDataBytes > 0)
      Stream.ReadBytes(&TempData[8], iDataBytes);
  }

  // in case we have no connection to a peer, queue the message
  if (!IsConnectedToOther())
  {
    if (TempData.GetCount() > 8)
      QueueOutgoingMessage(tm, uiSystemID, uiMsgID, &TempData[8], TempData.GetCount() - 8);
    else
      QueueOutgoingMessage(tm, uiSystemID, uiMsgID, nullptr, 0);
  }
  else
  {
    // when we do have a connection, just send the message out
    Transmit(tm, &TempData[0], TempData.GetCount());
  }
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void wdTelemetry::CloseConnection()
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  s_bConnectedToServer = false;
  s_bConnectedToClient = false;
  s_ConnectionMode = None;
  s_uiServerID = 0;
  g_pConnectionToServer = nullptr;

  StopTelemetryThread();

  // prevent other threads from interfering
  WD_LOCK(GetTelemetryMutex());

  UpdateNetwork();
  wdThreadUtils::Sleep(wdTime::Milliseconds(10));

  if (g_pHost)
  {
    // send all peers that we are disconnecting
    for (wdUInt32 i = (wdUInt32)g_pHost->connectedPeers; i > 0; --i)
      enet_peer_disconnect(&g_pHost->peers[i - 1], 0);

    // process the network messages (e.g. send the disconnect messages)
    UpdateNetwork();
    wdThreadUtils::Sleep(wdTime::Milliseconds(10));
  }

  // finally close the network connection
  if (g_pHost)
  {
    enet_host_destroy(g_pHost);
    g_pHost = nullptr;
  }

  if (g_bInitialized)
  {
    enet_deinitialize();
    g_bInitialized = false;
  }

  // if there are any queued messages, throw them away
  for (auto it = s_SystemMessages.GetIterator(); it.IsValid(); ++it)
  {
    it.Value().m_IncomingQueue.Clear();
    it.Value().m_OutgoingQueue.Clear();
  }
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}



WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_Telemetry);
