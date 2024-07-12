#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Profiling/Profiling.h>

void nsTelemetry::QueueOutgoingMessage(TransmitMode tm, nsUInt32 uiSystemID, nsUInt32 uiMsgID, const void* pData, nsUInt32 uiDataBytes)
{
  // unreliable packages can just be dropped
  if (tm == nsTelemetry::Unreliable)
    return;

  NS_LOCK(GetTelemetryMutex());

  // add a new message to the queue
  MessageQueue& Queue = s_SystemMessages[uiSystemID];
  Queue.m_OutgoingQueue.PushBack();

  // and fill it out properly
  nsTelemetryMessage& msg = Queue.m_OutgoingQueue.PeekBack();
  msg.SetMessageID(uiSystemID, uiMsgID);

  if (uiDataBytes > 0)
  {
    msg.GetWriter().WriteBytes(pData, uiDataBytes).IgnoreResult();
  }

  // if our outgoing queue has grown too large, dismiss older messages
  if (Queue.m_OutgoingQueue.GetCount() > Queue.m_uiMaxQueuedOutgoing)
    Queue.m_OutgoingQueue.PopFront(Queue.m_OutgoingQueue.GetCount() - Queue.m_uiMaxQueuedOutgoing);
}

void nsTelemetry::FlushOutgoingQueues()
{
  static bool bRecursion = false;

  if (bRecursion)
    return;

  // if there is no connection to anyone (yet), don't do anything
  if (!IsConnectedToOther())
    return;

  bRecursion = true;

  NS_LOCK(GetTelemetryMutex());

  // go through all system types
  for (auto it = s_SystemMessages.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value().m_OutgoingQueue.IsEmpty())
      continue;

    const nsUInt32 uiCurCount = it.Value().m_OutgoingQueue.GetCount();

    // send all messages that are queued for this system
    for (nsUInt32 i = 0; i < uiCurCount; ++i)
      Send(nsTelemetry::Reliable, it.Value().m_OutgoingQueue[i]); // Send() will already update the network

    // check that they have not been queue again
    NS_ASSERT_DEV(it.Value().m_OutgoingQueue.GetCount() == uiCurCount, "Implementation Error: When queued messages are flushed, they should not get queued again.");

    it.Value().m_OutgoingQueue.Clear();
  }

  bRecursion = false;
}


nsResult nsTelemetry::ConnectToServer(nsStringView sConnectTo)
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  return OpenConnection(Client, sConnectTo);
#else
  nsLog::SeriousWarning("Enet is not compiled into this build, nsTelemetry::ConnectToServer() will be ignored.");
  return NS_FAILURE;
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void nsTelemetry::CreateServer()
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  if (OpenConnection(Server).Failed())
  {
    nsLog::Error("nsTelemetry: Failed to open a connection as a server.");
    s_ConnectionMode = ConnectionMode::None;
  }
#else
  nsLog::SeriousWarning("Enet is not compiled into this build, nsTelemetry::CreateServer() will be ignored.");
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void nsTelemetry::AcceptMessagesForSystem(nsUInt32 uiSystemID, bool bAccept, ProcessMessagesCallback callback, void* pPassThrough)
{
  NS_LOCK(GetTelemetryMutex());

  s_SystemMessages[uiSystemID].m_bAcceptMessages = bAccept;
  s_SystemMessages[uiSystemID].m_Callback = callback;
  s_SystemMessages[uiSystemID].m_pPassThrough = pPassThrough;
}

void nsTelemetry::PerFrameUpdate()
{
  NS_PROFILE_SCOPE("Telemetry.PerFrameUpdate");
  NS_LOCK(GetTelemetryMutex());

  // Call each callback to process the incoming messages
  for (auto it = s_SystemMessages.GetIterator(); it.IsValid(); ++it)
  {
    if (!it.Value().m_IncomingQueue.IsEmpty() && it.Value().m_Callback)
      it.Value().m_Callback(it.Value().m_pPassThrough);
  }

  TelemetryEventData e;
  e.m_EventType = TelemetryEventData::PerFrameUpdate;

  const bool bAllowUpdate = s_bAllowNetworkUpdate;
  s_bAllowNetworkUpdate = false;
  s_TelemetryEvents.Broadcast(e);
  s_bAllowNetworkUpdate = bAllowUpdate;
}

void nsTelemetry::SetOutgoingQueueSize(nsUInt32 uiSystemID, nsUInt16 uiMaxQueued)
{
  NS_LOCK(GetTelemetryMutex());

  s_SystemMessages[uiSystemID].m_uiMaxQueuedOutgoing = uiMaxQueued;
}


bool nsTelemetry::IsConnectedToOther()
{
  return ((s_ConnectionMode == Client && IsConnectedToServer()) || (s_ConnectionMode == Server && IsConnectedToClient()));
}

void nsTelemetry::Broadcast(TransmitMode tm, nsUInt32 uiSystemID, nsUInt32 uiMsgID, const void* pData, nsUInt32 uiDataBytes)
{
  if (s_ConnectionMode != nsTelemetry::Server)
    return;

  Send(tm, uiSystemID, uiMsgID, pData, uiDataBytes);
}

void nsTelemetry::Broadcast(TransmitMode tm, nsUInt32 uiSystemID, nsUInt32 uiMsgID, nsStreamReader& inout_stream, nsInt32 iDataBytes)
{
  if (s_ConnectionMode != nsTelemetry::Server)
    return;

  Send(tm, uiSystemID, uiMsgID, inout_stream, iDataBytes);
}

void nsTelemetry::Broadcast(TransmitMode tm, nsTelemetryMessage& ref_msg)
{
  if (s_ConnectionMode != nsTelemetry::Server)
    return;

  Send(tm, ref_msg);
}

void nsTelemetry::SendToServer(nsUInt32 uiSystemID, nsUInt32 uiMsgID, const void* pData, nsUInt32 uiDataBytes)
{
  if (s_ConnectionMode != nsTelemetry::Client)
    return;

  Send(nsTelemetry::Reliable, uiSystemID, uiMsgID, pData, uiDataBytes);
}

void nsTelemetry::SendToServer(nsUInt32 uiSystemID, nsUInt32 uiMsgID, nsStreamReader& inout_stream, nsInt32 iDataBytes)
{
  if (s_ConnectionMode != nsTelemetry::Client)
    return;

  Send(nsTelemetry::Reliable, uiSystemID, uiMsgID, inout_stream, iDataBytes);
}

void nsTelemetry::SendToServer(nsTelemetryMessage& ref_msg)
{
  if (s_ConnectionMode != nsTelemetry::Client)
    return;

  Send(nsTelemetry::Reliable, ref_msg);
}

void nsTelemetry::Send(TransmitMode tm, nsTelemetryMessage& msg)
{
  Send(tm, msg.GetSystemID(), msg.GetMessageID(), msg.GetReader(), (nsInt32)msg.m_Storage.GetStorageSize32());
}
