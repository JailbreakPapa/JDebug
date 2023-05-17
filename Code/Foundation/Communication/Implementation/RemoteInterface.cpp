#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/RemoteInterface.h>
#include <Foundation/Utilities/ConversionUtils.h>

wdRemoteInterface::~wdRemoteInterface()
{
  // unfortunately we cannot do that ourselves here, because ShutdownConnection() calls virtual functions
  // and this object is already partially destructed here (derived class is already shut down)
  WD_ASSERT_DEV(m_RemoteMode == wdRemoteMode::None, "wdRemoteInterface::ShutdownConnection() has to be called before destroying the interface");
}

wdResult wdRemoteInterface::CreateConnection(wdUInt32 uiConnectionToken, wdRemoteMode mode, const char* szServerAddress, bool bStartUpdateThread)
{
  wdUInt32 uiPrevID = m_uiApplicationID;
  ShutdownConnection();
  m_uiApplicationID = uiPrevID;

  WD_LOCK(GetMutex());

  m_uiConnectionToken = uiConnectionToken;
  m_sServerAddress = szServerAddress;

  if (m_uiApplicationID == 0)
  {
    // create a 'unique' ID to identify this application
    m_uiApplicationID = (wdUInt32)wdTime::Now().GetSeconds();
  }

  if (InternalCreateConnection(mode, szServerAddress).Failed())
  {
    ShutdownConnection();
    return WD_FAILURE;
  }

  m_RemoteMode = mode;

  UpdateRemoteInterface();

  if (bStartUpdateThread)
  {
    StartUpdateThread();
  }

  return WD_SUCCESS;
}

wdResult wdRemoteInterface::StartServer(wdUInt32 uiConnectionToken, const char* szAddress, bool bStartUpdateThread /*= true*/)
{
  return CreateConnection(uiConnectionToken, wdRemoteMode::Server, szAddress, bStartUpdateThread);
}

wdResult wdRemoteInterface::ConnectToServer(wdUInt32 uiConnectionToken, const char* szAddress, bool bStartUpdateThread /*= true*/)
{
  return CreateConnection(uiConnectionToken, wdRemoteMode::Client, szAddress, bStartUpdateThread);
}

wdResult wdRemoteInterface::WaitForConnectionToServer(wdTime timeout /*= wdTime::Seconds(10)*/)
{
  if (m_RemoteMode != wdRemoteMode::Client)
    return WD_FAILURE;

  const wdTime tStart = wdTime::Now();

  while (true)
  {
    UpdateRemoteInterface();

    if (IsConnectedToServer())
      return WD_SUCCESS;

    if (timeout.GetSeconds() != 0)
    {
      if (wdTime::Now() - tStart > timeout)
        return WD_FAILURE;
    }

    wdThreadUtils::Sleep(wdTime::Milliseconds(10));
  }
}

void wdRemoteInterface::ShutdownConnection()
{
  StopUpdateThread();

  WD_LOCK(GetMutex());

  if (m_RemoteMode != wdRemoteMode::None)
  {
    InternalShutdownConnection();

    m_RemoteMode = wdRemoteMode::None;
    m_uiApplicationID = 0;
    m_uiConnectionToken = 0;
  }
}

void wdRemoteInterface::UpdatePingToServer()
{
  if (m_RemoteMode == wdRemoteMode::Server)
  {
    WD_LOCK(GetMutex());
    m_PingToServer = InternalGetPingToServer();
  }
}

void wdRemoteInterface::UpdateRemoteInterface()
{
  WD_LOCK(GetMutex());

  InternalUpdateRemoteInterface();
}

wdResult wdRemoteInterface::Transmit(wdRemoteTransmitMode tm, const wdArrayPtr<const wdUInt8>& data)
{
  if (m_RemoteMode == wdRemoteMode::None)
    return WD_FAILURE;

  WD_LOCK(GetMutex());

  if (InternalTransmit(tm, data).Failed())
    return WD_FAILURE;

  // make sure the message is processed immediately
  UpdateRemoteInterface();

  return WD_SUCCESS;
}


void wdRemoteInterface::Send(wdUInt32 uiSystemID, wdUInt32 uiMsgID)
{
  Send(wdRemoteTransmitMode::Reliable, uiSystemID, uiMsgID, wdArrayPtr<const wdUInt8>());
}

void wdRemoteInterface::Send(wdRemoteTransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const wdArrayPtr<const wdUInt8>& data)
{
  if (m_RemoteMode == wdRemoteMode::None)
    return;

  // if (!IsConnectedToOther())
  //  return;

  m_TempSendBuffer.SetCountUninitialized(12 + data.GetCount());
  *((wdUInt32*)&m_TempSendBuffer[0]) = m_uiApplicationID;
  *((wdUInt32*)&m_TempSendBuffer[4]) = uiSystemID;
  *((wdUInt32*)&m_TempSendBuffer[8]) = uiMsgID;

  if (!data.IsEmpty())
  {
    wdUInt8* pCopyDst = &m_TempSendBuffer[12];
    wdMemoryUtils::Copy(pCopyDst, data.GetPtr(), data.GetCount());
  }

  Transmit(tm, m_TempSendBuffer).IgnoreResult();
}

void wdRemoteInterface::Send(wdRemoteTransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const void* pData /*= nullptr*/, wdUInt32 uiDataBytes /*= 0*/)
{
  Send(tm, uiSystemID, uiMsgID, wdArrayPtr<const wdUInt8>(reinterpret_cast<const wdUInt8*>(pData), uiDataBytes));
}

void wdRemoteInterface::Send(wdRemoteTransmitMode tm, wdRemoteMessage& ref_msg)
{
  Send(tm, ref_msg.GetSystemID(), ref_msg.GetMessageID(), ref_msg.m_Storage);
}

void wdRemoteInterface::Send(wdRemoteTransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const wdContiguousMemoryStreamStorage& data)
{
  if (m_RemoteMode == wdRemoteMode::None)
    return;

  // if (!IsConnectedToOther())
  //  return;

  wdArrayPtr<const wdUInt8> range = {data.GetData(), data.GetStorageSize32()};

  m_TempSendBuffer.SetCountUninitialized(12 + range.GetCount());
  *((wdUInt32*)&m_TempSendBuffer[0]) = m_uiApplicationID;
  *((wdUInt32*)&m_TempSendBuffer[4]) = uiSystemID;
  *((wdUInt32*)&m_TempSendBuffer[8]) = uiMsgID;

  if (!range.IsEmpty())
  {
    wdUInt8* pCopyDst = &m_TempSendBuffer[12];
    wdMemoryUtils::Copy(pCopyDst, range.GetPtr(), range.GetCount());
  }

  Transmit(tm, m_TempSendBuffer).IgnoreResult();
}

void wdRemoteInterface::SetMessageHandler(wdUInt32 uiSystemID, wdRemoteMessageHandler messageHandler)
{
  m_MessageQueues[uiSystemID].m_MessageHandler = messageHandler;
}

wdUInt32 wdRemoteInterface::ExecuteMessageHandlers(wdUInt32 uiSystem)
{
  WD_LOCK(m_Mutex);

  return ExecuteMessageHandlersForQueue(m_MessageQueues[uiSystem]);
}

wdUInt32 wdRemoteInterface::ExecuteAllMessageHandlers()
{
  WD_LOCK(m_Mutex);

  wdUInt32 ret = 0;
  for (auto it = m_MessageQueues.GetIterator(); it.IsValid(); ++it)
  {
    ret += ExecuteMessageHandlersForQueue(it.Value());
  }

  return ret;
}

wdUInt32 wdRemoteInterface::ExecuteMessageHandlersForQueue(wdRemoteMessageQueue& queue)
{
  queue.m_MessageQueueIn.Swap(queue.m_MessageQueueOut);
  const wdUInt32 ret = queue.m_MessageQueueOut.GetCount();


  if (queue.m_MessageHandler.IsValid())
  {
    for (auto& msg : queue.m_MessageQueueOut)
    {
      queue.m_MessageHandler(msg);
    }
  }

  queue.m_MessageQueueOut.Clear();

  return ret;
}

void wdRemoteInterface::StartUpdateThread()
{
  StopUpdateThread();

  if (m_pUpdateThread == nullptr)
  {
    WD_LOCK(m_Mutex);

    m_pUpdateThread = WD_DEFAULT_NEW(wdRemoteThread);
    m_pUpdateThread->m_pRemoteInterface = this;
    m_pUpdateThread->Start();
  }
}

void wdRemoteInterface::StopUpdateThread()
{
  if (m_pUpdateThread != nullptr)
  {
    m_pUpdateThread->m_bKeepRunning = false;
    m_pUpdateThread->Join();

    WD_LOCK(m_Mutex);
    WD_DEFAULT_DELETE(m_pUpdateThread);
  }
}


void wdRemoteInterface::ReportConnectionToServer(wdUInt32 uiServerID)
{
  if (m_uiConnectedToServerWithID == uiServerID)
    return;

  m_uiConnectedToServerWithID = uiServerID;

  wdRemoteEvent e;
  e.m_Type = wdRemoteEvent::ConnectedToServer;
  e.m_uiOtherAppID = uiServerID;
  m_RemoteEvents.Broadcast(e);
}


void wdRemoteInterface::ReportConnectionToClient(wdUInt32 uiApplicationID)
{
  m_iConnectionsToClients++;

  wdRemoteEvent e;
  e.m_Type = wdRemoteEvent::ConnectedToClient;
  e.m_uiOtherAppID = uiApplicationID;
  m_RemoteEvents.Broadcast(e);
}

void wdRemoteInterface::ReportDisconnectedFromServer()
{
  m_uiConnectedToServerWithID = 0;

  wdRemoteEvent e;
  e.m_Type = wdRemoteEvent::DisconnectedFromServer;
  e.m_uiOtherAppID = m_uiConnectedToServerWithID;
  m_RemoteEvents.Broadcast(e);
}

void wdRemoteInterface::ReportDisconnectedFromClient(wdUInt32 uiApplicationID)
{
  m_iConnectionsToClients--;

  wdRemoteEvent e;
  e.m_Type = wdRemoteEvent::DisconnectedFromClient;
  e.m_uiOtherAppID = uiApplicationID;
  m_RemoteEvents.Broadcast(e);
}


void wdRemoteInterface::ReportMessage(wdUInt32 uiApplicationID, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const wdArrayPtr<const wdUInt8>& data)
{
  WD_LOCK(m_Mutex);

  auto& queue = m_MessageQueues[uiSystemID];

  // discard messages for which we have no message handler
  if (!queue.m_MessageHandler.IsValid())
    return;

  // store the data for later
  auto& msg = queue.m_MessageQueueIn.ExpandAndGetRef();
  msg.m_uiApplicationID = uiApplicationID;
  msg.SetMessageID(uiSystemID, uiMsgID);
  msg.GetWriter().WriteBytes(data.GetPtr(), data.GetCount()).IgnoreResult();
}

wdResult wdRemoteInterface::DetermineTargetAddress(const char* szConnectTo, wdUInt32& out_IP, wdUInt16& out_Port)
{
  out_IP = 0;
  out_Port = 0;

  wdStringBuilder sConnectTo = szConnectTo;

  const char* szColon = sConnectTo.FindLastSubString(":");
  if (szColon != nullptr)
  {
    sConnectTo.Shrink(0, wdStringUtils::GetStringElementCount(szColon));

    wdStringBuilder sPort = szColon + 1;

    wdInt32 tmp;
    if (wdConversionUtils::StringToInt(sPort, tmp).Succeeded())
      out_Port = static_cast<wdUInt16>(tmp);
  }

  wdInt32 ip1 = 0;
  wdInt32 ip2 = 0;
  wdInt32 ip3 = 0;
  wdInt32 ip4 = 0;

  if (sConnectTo.IsEmpty() || sConnectTo.IsEqual_NoCase("localhost"))
  {
    ip1 = 127;
    ip2 = 0;
    ip3 = 0;
    ip4 = 1;
  }
  else if (sConnectTo.FindSubString(".") != nullptr)
  {
    wdHybridArray<wdString, 8> IP;
    sConnectTo.Split(false, IP, ".");

    if (IP.GetCount() != 4)
      return WD_FAILURE;

    if (wdConversionUtils::StringToInt(IP[0], ip1).Failed())
      return WD_FAILURE;
    if (wdConversionUtils::StringToInt(IP[1], ip2).Failed())
      return WD_FAILURE;
    if (wdConversionUtils::StringToInt(IP[2], ip3).Failed())
      return WD_FAILURE;
    if (wdConversionUtils::StringToInt(IP[3], ip4).Failed())
      return WD_FAILURE;
  }
  else
  {
    return WD_FAILURE;
  }

  out_IP = ((ip1 & 0xFF) | (ip2 & 0xFF) << 8 | (ip3 & 0xFF) << 16 | (ip4 & 0xFF) << 24);
  return WD_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

wdRemoteThread::wdRemoteThread()
  : wdThread("wdRemoteThread")
{
}

wdUInt32 wdRemoteThread::Run()
{
  wdTime lastPing;

  while (m_bKeepRunning && m_pRemoteInterface)
  {
    m_pRemoteInterface->UpdateRemoteInterface();

    // Send a Ping every once in a while
    if (m_pRemoteInterface->GetRemoteMode() == wdRemoteMode::Client)
    {
      wdTime tNow = wdTime::Now();

      if (tNow - lastPing > wdTime::Milliseconds(500))
      {
        lastPing = tNow;

        m_pRemoteInterface->UpdatePingToServer();
      }
    }

    wdThreadUtils::Sleep(wdTime::Milliseconds(10));
  }

  return 0;
}

WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_RemoteInterface);
