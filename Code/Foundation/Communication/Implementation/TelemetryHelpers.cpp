#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Telemetry.h>
#include <Foundation/Profiling/Profiling.h>

void wdTelemetry::QueueOutgoingMessage(TransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const void* pData, wdUInt32 uiDataBytes)
{
  // unreliable packages can just be dropped
  if (tm == wdTelemetry::Unreliable)
    return;

  WD_LOCK(GetTelemetryMutex());

  // add a new message to the queue
  MessageQueue& Queue = s_SystemMessages[uiSystemID];
  Queue.m_OutgoingQueue.PushBack();

  // and fill it out properly
  wdTelemetryMessage& msg = Queue.m_OutgoingQueue.PeekBack();
  msg.SetMessageID(uiSystemID, uiMsgID);

  if (uiDataBytes > 0)
  {
    msg.GetWriter().WriteBytes(pData, uiDataBytes).IgnoreResult();
  }

  // if our outgoing queue has grown too large, dismiss older messages
  if (Queue.m_OutgoingQueue.GetCount() > Queue.m_uiMaxQueuedOutgoing)
    Queue.m_OutgoingQueue.PopFront(Queue.m_OutgoingQueue.GetCount() - Queue.m_uiMaxQueuedOutgoing);
}

void wdTelemetry::FlushOutgoingQueues()
{
  static bool bRecursion = false;

  if (bRecursion)
    return;

  // if there is no connection to anyone (yet), don't do anything
  if (!IsConnectedToOther())
    return;

  bRecursion = true;

  WD_LOCK(GetTelemetryMutex());

  // go through all system types
  for (auto it = s_SystemMessages.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value().m_OutgoingQueue.IsEmpty())
      continue;

    const wdUInt32 uiCurCount = it.Value().m_OutgoingQueue.GetCount();

    // send all messages that are queued for this system
    for (wdUInt32 i = 0; i < uiCurCount; ++i)
      Send(wdTelemetry::Reliable, it.Value().m_OutgoingQueue[i]); // Send() will already update the network

    // check that they have not been queue again
    WD_ASSERT_DEV(it.Value().m_OutgoingQueue.GetCount() == uiCurCount, "Implementation Error: When queued messages are flushed, they should not get queued again.");

    it.Value().m_OutgoingQueue.Clear();
  }

  bRecursion = false;
}


wdResult wdTelemetry::ConnectToServer(const char* szConnectTo)
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  return OpenConnection(Client, szConnectTo);
#else
  wdLog::SeriousWarning("Enet is not compiled into this build, wdTelemetry::ConnectToServer() will be ignored.");
  return WD_FAILURE;
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void wdTelemetry::CreateServer()
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  if (OpenConnection(Server).Failed())
  {
    wdLog::Error("wdTelemetry: Failed to open a connection as a server.");
    s_ConnectionMode = ConnectionMode::None;
  }
#else
  wdLog::SeriousWarning("Enet is not compiled into this build, wdTelemetry::CreateServer() will be ignored.");
#endif // BUILDSYSTEM_ENABLE_ENET_SUPPORT
}

void wdTelemetry::AcceptMessagesForSystem(wdUInt32 uiSystemID, bool bAccept, ProcessMessagesCallback callback, void* pPassThrough)
{
  WD_LOCK(GetTelemetryMutex());

  s_SystemMessages[uiSystemID].m_bAcceptMessages = bAccept;
  s_SystemMessages[uiSystemID].m_Callback = callback;
  s_SystemMessages[uiSystemID].m_pPassThrough = pPassThrough;
}

void wdTelemetry::PerFrameUpdate()
{
  WD_PROFILE_SCOPE("Telemetry.PerFrameUpdate");
  WD_LOCK(GetTelemetryMutex());

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

void wdTelemetry::SetOutgoingQueueSize(wdUInt32 uiSystemID, wdUInt16 uiMaxQueued)
{
  WD_LOCK(GetTelemetryMutex());

  s_SystemMessages[uiSystemID].m_uiMaxQueuedOutgoing = uiMaxQueued;
}


bool wdTelemetry::IsConnectedToOther()
{
  return ((s_ConnectionMode == Client && IsConnectedToServer()) || (s_ConnectionMode == Server && IsConnectedToClient()));
}

void wdTelemetry::Broadcast(TransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, const void* pData, wdUInt32 uiDataBytes)
{
  if (s_ConnectionMode != wdTelemetry::Server)
    return;

  Send(tm, uiSystemID, uiMsgID, pData, uiDataBytes);
}

void wdTelemetry::Broadcast(TransmitMode tm, wdUInt32 uiSystemID, wdUInt32 uiMsgID, wdStreamReader& inout_stream, wdInt32 iDataBytes)
{
  if (s_ConnectionMode != wdTelemetry::Server)
    return;

  Send(tm, uiSystemID, uiMsgID, inout_stream, iDataBytes);
}

void wdTelemetry::Broadcast(TransmitMode tm, wdTelemetryMessage& ref_msg)
{
  if (s_ConnectionMode != wdTelemetry::Server)
    return;

  Send(tm, ref_msg);
}

void wdTelemetry::SendToServer(wdUInt32 uiSystemID, wdUInt32 uiMsgID, const void* pData, wdUInt32 uiDataBytes)
{
  if (s_ConnectionMode != wdTelemetry::Client)
    return;

  Send(wdTelemetry::Reliable, uiSystemID, uiMsgID, pData, uiDataBytes);
}

void wdTelemetry::SendToServer(wdUInt32 uiSystemID, wdUInt32 uiMsgID, wdStreamReader& inout_stream, wdInt32 iDataBytes)
{
  if (s_ConnectionMode != wdTelemetry::Client)
    return;

  Send(wdTelemetry::Reliable, uiSystemID, uiMsgID, inout_stream, iDataBytes);
}

void wdTelemetry::SendToServer(wdTelemetryMessage& ref_msg)
{
  if (s_ConnectionMode != wdTelemetry::Client)
    return;

  Send(wdTelemetry::Reliable, ref_msg);
}

void wdTelemetry::Send(TransmitMode tm, wdTelemetryMessage& msg)
{
  Send(tm, msg.GetSystemID(), msg.GetMessageID(), msg.GetReader(), (wdInt32)msg.m_Storage.GetStorageSize32());
}



WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_TelemetryHelpers);
