#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Implementation/IpcChannelEnet.h>
#include <Foundation/Communication/Implementation/MessageLoop.h>
#include <Foundation/Communication/IpcChannel.h>
#include <Foundation/Communication/RemoteMessage.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/ReflectionSerializer.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
#  include <Foundation/Communication/Implementation/Win/PipeChannel_win.h>
#elif WD_ENABLED(WD_PLATFORM_LINUX)
#  include <Foundation/Communication/Implementation/Linux/PipeChannel_linux.h>
#endif

wdIpcChannel::wdIpcChannel(const char* szAddress, Mode::Enum mode)
  : m_Mode(mode)
  , m_pOwner(wdMessageLoop::GetSingleton())
{
}

wdIpcChannel::~wdIpcChannel()
{
  wdDeque<wdUniquePtr<wdProcessMessage>> messages;
  SwapWorkQueue(messages);
  messages.Clear();

  m_pOwner->RemoveChannel(this);
}

wdIpcChannel* wdIpcChannel::CreatePipeChannel(const char* szAddress, Mode::Enum mode)
{
  if (wdStringUtils::IsNullOrEmpty(szAddress) || wdStringUtils::GetStringElementCount(szAddress) > 200)
  {
    wdLog::Error("Failed co create pipe '{0}', name is not valid", szAddress);
    return nullptr;
  }

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
  return WD_DEFAULT_NEW(wdPipeChannel_win, szAddress, mode);
#elif WD_ENABLED(WD_PLATFORM_LINUX)
  return WD_DEFAULT_NEW(wdPipeChannel_linux, szAddress, mode);
#else
  WD_ASSERT_NOT_IMPLEMENTED;
  return nullptr;
#endif
}


wdIpcChannel* wdIpcChannel::CreateNetworkChannel(const char* szAddress, Mode::Enum mode)
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  return WD_DEFAULT_NEW(wdIpcChannelEnet, szAddress, mode);
#else
  WD_ASSERT_NOT_IMPLEMENTED;
  return nullptr;
#endif
}

void wdIpcChannel::Connect()
{
  WD_LOCK(m_pOwner->m_TasksMutex);
  m_pOwner->m_ConnectQueue.PushBack(this);
  m_pOwner->WakeUp();
}


void wdIpcChannel::Disconnect()
{
  WD_LOCK(m_pOwner->m_TasksMutex);
  m_pOwner->m_DisconnectQueue.PushBack(this);
  m_pOwner->WakeUp();
}

bool wdIpcChannel::Send(wdProcessMessage* pMsg)
{
  {
    WD_LOCK(m_OutputQueueMutex);
    wdMemoryStreamStorageInterface& storage = m_OutputQueue.ExpandAndGetRef();
    wdMemoryStreamWriter writer(&storage);
    wdUInt32 uiSize = 0;
    wdUInt32 uiMagic = MAGIC_VALUE;
    writer << uiMagic;
    writer << uiSize;
    WD_ASSERT_DEBUG(storage.GetStorageSize32() == HEADER_SIZE, "Magic value and size should have written HEADER_SIZE bytes.");
    wdReflectionSerializer::WriteObjectToBinary(writer, pMsg->GetDynamicRTTI(), pMsg);

    // reset to the beginning and write the stored size again
    writer.SetWritePosition(4);
    writer << storage.GetStorageSize32();
  }
  if (m_bConnected)
  {
    if (NeedWakeup())
    {
      WD_LOCK(m_pOwner->m_TasksMutex);
      if (!m_pOwner->m_SendQueue.Contains(this))
        m_pOwner->m_SendQueue.PushBack(this);
      m_pOwner->WakeUp();
      return true;
    }
  }
  return false;
}

bool wdIpcChannel::ProcessMessages()
{
  wdDeque<wdUniquePtr<wdProcessMessage>> messages;
  SwapWorkQueue(messages);
  if (messages.IsEmpty())
  {
    return false;
  }

  while (!messages.IsEmpty())
  {
    wdUniquePtr<wdProcessMessage> msg = std::move(messages.PeekFront());
    messages.PopFront();
    m_MessageEvent.Broadcast(msg.Borrow());
  }

  return true;
}

void wdIpcChannel::WaitForMessages()
{
  if (m_bConnected)
  {
    m_IncomingMessages.WaitForSignal();
    ProcessMessages();
  }
}

wdResult wdIpcChannel::WaitForMessages(wdTime timeout)
{
  if (m_bConnected)
  {
    if (m_IncomingMessages.WaitForSignal(timeout) == wdThreadSignal::WaitResult::Timeout)
    {
      return WD_FAILURE;
    }
    ProcessMessages();
  }

  return WD_SUCCESS;
}

void wdIpcChannel::ReceiveMessageData(wdArrayPtr<const wdUInt8> data)
{
  wdArrayPtr<const wdUInt8> remainingData = data;
  while (true)
  {
    if (m_MessageAccumulator.GetCount() < HEADER_SIZE)
    {
      if (remainingData.GetCount() + m_MessageAccumulator.GetCount() < HEADER_SIZE)
      {
        m_MessageAccumulator.PushBackRange(remainingData);
        return;
      }
      else
      {
        wdUInt32 uiRemainingHeaderData = HEADER_SIZE - m_MessageAccumulator.GetCount();
        wdArrayPtr<const wdUInt8> headerData = remainingData.GetSubArray(0, uiRemainingHeaderData);
        m_MessageAccumulator.PushBackRange(headerData);
        WD_ASSERT_DEBUG(m_MessageAccumulator.GetCount() == HEADER_SIZE, "We should have a full header now.");
        remainingData = remainingData.GetSubArray(uiRemainingHeaderData);
      }
    }

    WD_ASSERT_DEBUG(m_MessageAccumulator.GetCount() >= HEADER_SIZE, "Header must be complete at this point.");
    if (remainingData.IsEmpty())
      return;

    // Read and verify header
    wdUInt32 uiMagic = *reinterpret_cast<const wdUInt32*>(m_MessageAccumulator.GetData());
    WD_IGNORE_UNUSED(uiMagic);
    WD_ASSERT_DEBUG(uiMagic == MAGIC_VALUE, "Message received with wrong magic value.");
    wdUInt32 uiMessageSize = *reinterpret_cast<const wdUInt32*>(m_MessageAccumulator.GetData() + 4);
    WD_ASSERT_DEBUG(uiMessageSize < MAX_MESSAGE_SIZE, "Message too big: {0}! Either the stream got corrupted or you need to increase MAX_MESSAGE_SIZE.", uiMessageSize);
    if (uiMessageSize > remainingData.GetCount() + m_MessageAccumulator.GetCount())
    {
      m_MessageAccumulator.PushBackRange(remainingData);
      return;
    }

    // Write missing data into message accumulator
    wdUInt32 remainingMessageData = uiMessageSize - m_MessageAccumulator.GetCount();
    wdArrayPtr<const wdUInt8> messageData = remainingData.GetSubArray(0, remainingMessageData);
    m_MessageAccumulator.PushBackRange(messageData);
    WD_ASSERT_DEBUG(m_MessageAccumulator.GetCount() == uiMessageSize, "");
    remainingData = remainingData.GetSubArray(remainingMessageData);

    {
      // Message complete, de-serialize
      wdRawMemoryStreamReader reader(m_MessageAccumulator.GetData() + HEADER_SIZE, uiMessageSize - HEADER_SIZE);
      const wdRTTI* pRtti = nullptr;

      wdProcessMessage* pMsg = (wdProcessMessage*)wdReflectionSerializer::ReadObjectFromBinary(reader, pRtti);
      wdUniquePtr<wdProcessMessage> msg(pMsg, wdFoundation::GetDefaultAllocator());
      if (msg != nullptr)
      {
        EnqueueMessage(std::move(msg));
      }
      else
      {
        wdLog::Error("Channel received invalid Message!");
      }
      m_MessageAccumulator.Clear();
    }
  }
}

void wdIpcChannel::EnqueueMessage(wdUniquePtr<wdProcessMessage>&& msg)
{
  {
    WD_LOCK(m_IncomingQueueMutex);
    m_IncomingQueue.PushBack(std::move(msg));
  }
  m_IncomingMessages.RaiseSignal();

  m_Events.Broadcast(wdIpcChannelEvent(wdIpcChannelEvent::NewMessages, this));
}

void wdIpcChannel::SwapWorkQueue(wdDeque<wdUniquePtr<wdProcessMessage>>& messages)
{
  WD_ASSERT_DEBUG(messages.IsEmpty(), "Swap target must be empty!");
  WD_LOCK(m_IncomingQueueMutex);
  if (m_IncomingQueue.IsEmpty())
    return;
  messages.Swap(m_IncomingQueue);
}

void wdIpcChannel::FlushPendingOperations()
{
  m_pOwner->WaitForMessages(-1, this);
}



WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_IpcChannel);
