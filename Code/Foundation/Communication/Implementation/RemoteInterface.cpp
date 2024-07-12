#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/RemoteInterface.h>
#include <Foundation/Utilities/ConversionUtils.h>

nsRemoteInterface::~nsRemoteInterface()
{
  // unfortunately we cannot do that ourselves here, because ShutdownConnection() calls virtual functions
  // and this object is already partially destructed here (derived class is already shut down)
  NS_ASSERT_DEV(m_RemoteMode == nsRemoteMode::None, "nsRemoteInterface::ShutdownConnection() has to be called before destroying the interface");
}

nsResult nsRemoteInterface::CreateConnection(nsUInt32 uiConnectionToken, nsRemoteMode mode, nsStringView sServerAddress, bool bStartUpdateThread)
{
  nsUInt32 uiPrevID = m_uiApplicationID;
  ShutdownConnection();
  m_uiApplicationID = uiPrevID;

  NS_LOCK(GetMutex());

  m_uiConnectionToken = uiConnectionToken;
  m_sServerAddress = sServerAddress;

  if (m_uiApplicationID == 0)
  {
    // create a 'unique' ID to identify this application
    m_uiApplicationID = (nsUInt32)nsTime::Now().GetSeconds();
  }

  if (InternalCreateConnection(mode, sServerAddress).Failed())
  {
    ShutdownConnection();
    return NS_FAILURE;
  }

  m_RemoteMode = mode;

  UpdateRemoteInterface();

  if (bStartUpdateThread)
  {
    StartUpdateThread();
  }

  return NS_SUCCESS;
}

nsResult nsRemoteInterface::StartServer(nsUInt32 uiConnectionToken, nsStringView sAddress, bool bStartUpdateThread /*= true*/)
{
  return CreateConnection(uiConnectionToken, nsRemoteMode::Server, sAddress, bStartUpdateThread);
}

nsResult nsRemoteInterface::ConnectToServer(nsUInt32 uiConnectionToken, nsStringView sAddress, bool bStartUpdateThread /*= true*/)
{
  return CreateConnection(uiConnectionToken, nsRemoteMode::Client, sAddress, bStartUpdateThread);
}

nsResult nsRemoteInterface::WaitForConnectionToServer(nsTime timeout /*= nsTime::MakeFromSeconds(10)*/)
{
  if (m_RemoteMode != nsRemoteMode::Client)
    return NS_FAILURE;

  const nsTime tStart = nsTime::Now();

  while (true)
  {
    UpdateRemoteInterface();

    if (IsConnectedToServer())
      return NS_SUCCESS;

    if (timeout.GetSeconds() != 0)
    {
      if (nsTime::Now() - tStart > timeout)
        return NS_FAILURE;
    }

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
  }
}

void nsRemoteInterface::ShutdownConnection()
{
  StopUpdateThread();

  NS_LOCK(GetMutex());

  if (m_RemoteMode != nsRemoteMode::None)
  {
    InternalShutdownConnection();

    m_RemoteMode = nsRemoteMode::None;
    m_uiApplicationID = 0;
    m_uiConnectionToken = 0;
  }
}

void nsRemoteInterface::UpdatePingToServer()
{
  if (m_RemoteMode == nsRemoteMode::Server)
  {
    NS_LOCK(GetMutex());
    m_PingToServer = InternalGetPingToServer();
  }
}

void nsRemoteInterface::UpdateRemoteInterface()
{
  NS_LOCK(GetMutex());

  InternalUpdateRemoteInterface();
}

nsResult nsRemoteInterface::Transmit(nsRemoteTransmitMode tm, const nsArrayPtr<const nsUInt8>& data)
{
  if (m_RemoteMode == nsRemoteMode::None)
    return NS_FAILURE;

  NS_LOCK(GetMutex());

  if (InternalTransmit(tm, data).Failed())
    return NS_FAILURE;

  // make sure the message is processed immediately
  UpdateRemoteInterface();

  return NS_SUCCESS;
}


void nsRemoteInterface::Send(nsUInt32 uiSystemID, nsUInt32 uiMsgID)
{
  Send(nsRemoteTransmitMode::Reliable, uiSystemID, uiMsgID, nsArrayPtr<const nsUInt8>());
}

void nsRemoteInterface::Send(nsRemoteTransmitMode tm, nsUInt32 uiSystemID, nsUInt32 uiMsgID, const nsArrayPtr<const nsUInt8>& data)
{
  if (m_RemoteMode == nsRemoteMode::None)
    return;

  // if (!IsConnectedToOther())
  //  return;

  m_TempSendBuffer.SetCountUninitialized(12 + data.GetCount());
  *((nsUInt32*)&m_TempSendBuffer[0]) = m_uiApplicationID;
  *((nsUInt32*)&m_TempSendBuffer[4]) = uiSystemID;
  *((nsUInt32*)&m_TempSendBuffer[8]) = uiMsgID;

  if (!data.IsEmpty())
  {
    nsUInt8* pCopyDst = &m_TempSendBuffer[12];
    nsMemoryUtils::Copy(pCopyDst, data.GetPtr(), data.GetCount());
  }

  Transmit(tm, m_TempSendBuffer).IgnoreResult();
}

void nsRemoteInterface::Send(nsRemoteTransmitMode tm, nsUInt32 uiSystemID, nsUInt32 uiMsgID, const void* pData /*= nullptr*/, nsUInt32 uiDataBytes /*= 0*/)
{
  Send(tm, uiSystemID, uiMsgID, nsArrayPtr<const nsUInt8>(reinterpret_cast<const nsUInt8*>(pData), uiDataBytes));
}

void nsRemoteInterface::Send(nsRemoteTransmitMode tm, nsRemoteMessage& ref_msg)
{
  Send(tm, ref_msg.GetSystemID(), ref_msg.GetMessageID(), ref_msg.m_Storage);
}

void nsRemoteInterface::Send(nsRemoteTransmitMode tm, nsUInt32 uiSystemID, nsUInt32 uiMsgID, const nsContiguousMemoryStreamStorage& data)
{
  if (m_RemoteMode == nsRemoteMode::None)
    return;

  // if (!IsConnectedToOther())
  //  return;

  nsArrayPtr<const nsUInt8> range = {data.GetData(), data.GetStorageSize32()};

  m_TempSendBuffer.SetCountUninitialized(12 + range.GetCount());
  *((nsUInt32*)&m_TempSendBuffer[0]) = m_uiApplicationID;
  *((nsUInt32*)&m_TempSendBuffer[4]) = uiSystemID;
  *((nsUInt32*)&m_TempSendBuffer[8]) = uiMsgID;

  if (!range.IsEmpty())
  {
    nsUInt8* pCopyDst = &m_TempSendBuffer[12];
    nsMemoryUtils::Copy(pCopyDst, range.GetPtr(), range.GetCount());
  }

  Transmit(tm, m_TempSendBuffer).IgnoreResult();
}

void nsRemoteInterface::SetMessageHandler(nsUInt32 uiSystemID, nsRemoteMessageHandler messageHandler)
{
  m_MessageQueues[uiSystemID].m_MessageHandler = messageHandler;
}

void nsRemoteInterface::SetUnhandledMessageHandler(nsRemoteMessageHandler messageHandler)
{
  m_UnhandledMessageHandler = messageHandler;
}

nsUInt32 nsRemoteInterface::ExecuteMessageHandlers(nsUInt32 uiSystem)
{
  NS_LOCK(m_Mutex);

  return ExecuteMessageHandlersForQueue(m_MessageQueues[uiSystem]);
}

nsUInt32 nsRemoteInterface::ExecuteAllMessageHandlers()
{
  NS_LOCK(m_Mutex);

  nsUInt32 ret = 0;
  for (auto it = m_MessageQueues.GetIterator(); it.IsValid(); ++it)
  {
    ret += ExecuteMessageHandlersForQueue(it.Value());
  }

  return ret;
}

nsUInt32 nsRemoteInterface::ExecuteMessageHandlersForQueue(nsRemoteMessageQueue& queue)
{
  queue.m_MessageQueueIn.Swap(queue.m_MessageQueueOut);
  const nsUInt32 ret = queue.m_MessageQueueOut.GetCount();

  if (queue.m_MessageHandler.IsValid())
  {
    for (auto& msg : queue.m_MessageQueueOut)
    {
      queue.m_MessageHandler(msg);
    }
  }
  else if (m_UnhandledMessageHandler.IsValid())
  {
    for (auto& msg : queue.m_MessageQueueOut)
    {
      m_UnhandledMessageHandler(msg);
    }
  }

  queue.m_MessageQueueOut.Clear();

  return ret;
}

void nsRemoteInterface::StartUpdateThread()
{
  StopUpdateThread();

  if (m_pUpdateThread == nullptr)
  {
    NS_LOCK(m_Mutex);

    m_pUpdateThread = NS_DEFAULT_NEW(nsRemoteThread);
    m_pUpdateThread->m_pRemoteInterface = this;
    m_pUpdateThread->Start();
  }
}

void nsRemoteInterface::StopUpdateThread()
{
  if (m_pUpdateThread != nullptr)
  {
    m_pUpdateThread->m_bKeepRunning = false;
    m_pUpdateThread->Join();

    NS_LOCK(m_Mutex);
    NS_DEFAULT_DELETE(m_pUpdateThread);
  }
}


void nsRemoteInterface::ReportConnectionToServer(nsUInt32 uiServerID)
{
  if (m_uiConnectedToServerWithID == uiServerID)
    return;

  m_uiConnectedToServerWithID = uiServerID;

  nsRemoteEvent e;
  e.m_Type = nsRemoteEvent::ConnectedToServer;
  e.m_uiOtherAppID = uiServerID;
  m_RemoteEvents.Broadcast(e);
}


void nsRemoteInterface::ReportConnectionToClient(nsUInt32 uiApplicationID)
{
  m_iConnectionsToClients++;

  nsRemoteEvent e;
  e.m_Type = nsRemoteEvent::ConnectedToClient;
  e.m_uiOtherAppID = uiApplicationID;
  m_RemoteEvents.Broadcast(e);
}

void nsRemoteInterface::ReportDisconnectedFromServer()
{
  m_uiConnectedToServerWithID = 0;

  nsRemoteEvent e;
  e.m_Type = nsRemoteEvent::DisconnectedFromServer;
  e.m_uiOtherAppID = m_uiConnectedToServerWithID;
  m_RemoteEvents.Broadcast(e);
}

void nsRemoteInterface::ReportDisconnectedFromClient(nsUInt32 uiApplicationID)
{
  m_iConnectionsToClients--;

  nsRemoteEvent e;
  e.m_Type = nsRemoteEvent::DisconnectedFromClient;
  e.m_uiOtherAppID = uiApplicationID;
  m_RemoteEvents.Broadcast(e);
}


void nsRemoteInterface::ReportMessage(nsUInt32 uiApplicationID, nsUInt32 uiSystemID, nsUInt32 uiMsgID, const nsArrayPtr<const nsUInt8>& data)
{
  NS_LOCK(m_Mutex);

  auto& queue = m_MessageQueues[uiSystemID];

  // store the data for later
  auto& msg = queue.m_MessageQueueIn.ExpandAndGetRef();
  msg.m_uiApplicationID = uiApplicationID;
  msg.SetMessageID(uiSystemID, uiMsgID);
  msg.GetWriter().WriteBytes(data.GetPtr(), data.GetCount()).IgnoreResult();
}

nsResult nsRemoteInterface::DetermineTargetAddress(nsStringView sConnectTo0, nsUInt32& out_IP, nsUInt16& out_Port)
{
  out_IP = 0;
  out_Port = 0;

  nsStringBuilder sConnectTo = sConnectTo0;

  const char* szColon = sConnectTo.FindLastSubString(":");
  if (szColon != nullptr)
  {
    sConnectTo.Shrink(0, nsStringUtils::GetStringElementCount(szColon));

    nsStringBuilder sPort = szColon + 1;

    nsInt32 tmp;
    if (nsConversionUtils::StringToInt(sPort, tmp).Succeeded())
      out_Port = static_cast<nsUInt16>(tmp);
  }

  nsInt32 ip1 = 0;
  nsInt32 ip2 = 0;
  nsInt32 ip3 = 0;
  nsInt32 ip4 = 0;

  if (sConnectTo.IsEmpty() || sConnectTo.IsEqual_NoCase("localhost"))
  {
    ip1 = 127;
    ip2 = 0;
    ip3 = 0;
    ip4 = 1;
  }
  else if (sConnectTo.FindSubString(".") != nullptr)
  {
    nsHybridArray<nsString, 8> IP;
    sConnectTo.Split(false, IP, ".");

    if (IP.GetCount() != 4)
      return NS_FAILURE;

    if (nsConversionUtils::StringToInt(IP[0], ip1).Failed())
      return NS_FAILURE;
    if (nsConversionUtils::StringToInt(IP[1], ip2).Failed())
      return NS_FAILURE;
    if (nsConversionUtils::StringToInt(IP[2], ip3).Failed())
      return NS_FAILURE;
    if (nsConversionUtils::StringToInt(IP[3], ip4).Failed())
      return NS_FAILURE;
  }
  else
  {
    return NS_FAILURE;
  }

  out_IP = ((ip1 & 0xFF) | (ip2 & 0xFF) << 8 | (ip3 & 0xFF) << 16 | (ip4 & 0xFF) << 24);
  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

nsRemoteThread::nsRemoteThread()
  : nsThread("nsRemoteThread")
{
}

nsUInt32 nsRemoteThread::Run()
{
  nsTime lastPing;

  while (m_bKeepRunning && m_pRemoteInterface)
  {
    m_pRemoteInterface->UpdateRemoteInterface();

    // Send a Ping every once in a while
    if (m_pRemoteInterface->GetRemoteMode() == nsRemoteMode::Client)
    {
      nsTime tNow = nsTime::Now();

      if (tNow - lastPing > nsTime::MakeFromMilliseconds(500))
      {
        lastPing = tNow;

        m_pRemoteInterface->UpdatePingToServer();
      }
    }

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
  }

  return 0;
}
