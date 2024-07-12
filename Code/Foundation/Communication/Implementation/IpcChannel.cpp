#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Implementation/IpcChannelEnet.h>
#include <Foundation/Communication/Implementation/MessageLoop.h>
#include <Foundation/Communication/IpcChannel.h>
#include <Foundation/Communication/RemoteMessage.h>
#include <Foundation/Logging/Log.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
#  include <Foundation/Platform/Win/PipeChannel_Win.h>
#elif NS_ENABLED(NS_PLATFORM_LINUX)
#  include <Foundation/Platform/Linux/PipeChannel_Linux.h>
#endif

NS_CHECK_AT_COMPILETIME((nsInt32)nsIpcChannel::ConnectionState::Disconnected == (nsInt32)nsIpcChannelEvent::Disconnected);
NS_CHECK_AT_COMPILETIME((nsInt32)nsIpcChannel::ConnectionState::Connecting == (nsInt32)nsIpcChannelEvent::Connecting);
NS_CHECK_AT_COMPILETIME((nsInt32)nsIpcChannel::ConnectionState::Connected == (nsInt32)nsIpcChannelEvent::Connected);

nsIpcChannel::nsIpcChannel(nsStringView sAddress, Mode::Enum mode)
  : m_sAddress(sAddress)
  , m_Mode(mode)
  , m_pOwner(nsMessageLoop::GetSingleton())
{
}

nsIpcChannel::~nsIpcChannel()
{


  m_pOwner->RemoveChannel(this);
}

nsInternal::NewInstance<nsIpcChannel> nsIpcChannel::CreatePipeChannel(nsStringView sAddress, Mode::Enum mode)
{
  if (sAddress.IsEmpty() || sAddress.GetElementCount() > 200)
  {
    nsLog::Error("Failed co create pipe '{0}', name is not valid", sAddress);
    return nullptr;
  }

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
  return NS_DEFAULT_NEW(nsPipeChannel_win, sAddress, mode);
#elif NS_ENABLED(NS_PLATFORM_LINUX)
  return NS_DEFAULT_NEW(nsPipeChannel_linux, sAddress, mode);
#else
  NS_ASSERT_NOT_IMPLEMENTED;
  return nullptr;
#endif
}


nsInternal::NewInstance<nsIpcChannel> nsIpcChannel::CreateNetworkChannel(nsStringView sAddress, Mode::Enum mode)
{
#ifdef BUILDSYSTEM_ENABLE_ENET_SUPPORT
  return NS_DEFAULT_NEW(nsIpcChannelEnet, sAddress, mode);
#else
  NS_ASSERT_NOT_IMPLEMENTED;
  return nullptr;
#endif
}

void nsIpcChannel::Connect()
{
  NS_LOCK(m_pOwner->m_TasksMutex);
  m_pOwner->m_ConnectQueue.PushBack(this);
  m_pOwner->WakeUp();
}


void nsIpcChannel::Disconnect()
{
  NS_LOCK(m_pOwner->m_TasksMutex);
  m_pOwner->m_DisconnectQueue.PushBack(this);
  m_pOwner->WakeUp();
}


bool nsIpcChannel::Send(nsArrayPtr<const nsUInt8> data)
{
  {
    NS_LOCK(m_OutputQueueMutex);
    nsMemoryStreamStorageInterface& storage = m_OutputQueue.ExpandAndGetRef();
    nsMemoryStreamWriter writer(&storage);
    nsUInt32 uiSize = data.GetCount() + HEADER_SIZE;
    nsUInt32 uiMagic = MAGIC_VALUE;
    writer << uiMagic;
    writer << uiSize;
    NS_ASSERT_DEBUG(storage.GetStorageSize32() == HEADER_SIZE, "Magic value and size should have written HEADER_SIZE bytes.");
    writer.WriteBytes(data.GetPtr(), data.GetCount()).AssertSuccess("Failed to write to in-memory buffer, out of memory?");
  }
  if (IsConnected())
  {
    NS_LOCK(m_pOwner->m_TasksMutex);
    if (!m_pOwner->m_SendQueue.Contains(this))
      m_pOwner->m_SendQueue.PushBack(this);
    if (NeedWakeup())
    {
      m_pOwner->WakeUp();
    }
    return true;
  }
  return false;
}

void nsIpcChannel::SetReceiveCallback(ReceiveCallback callback)
{
  NS_LOCK(m_ReceiveCallbackMutex);
  m_ReceiveCallback = callback;
}

nsResult nsIpcChannel::WaitForMessages(nsTime timeout)
{
  if (IsConnected())
  {
    if (timeout == nsTime::MakeZero())
    {
      m_IncomingMessages.WaitForSignal();
    }
    else if (m_IncomingMessages.WaitForSignal(timeout) == nsThreadSignal::WaitResult::Timeout)
    {
      return NS_FAILURE;
    }
  }
  return NS_SUCCESS;
}

void nsIpcChannel::SetConnectionState(nsEnum<nsIpcChannel::ConnectionState> state)
{
  const nsEnum<nsIpcChannel::ConnectionState> oldValue = m_iConnectionState.Set(state);

  if (state != oldValue)
  {
    m_Events.Broadcast(nsIpcChannelEvent((nsIpcChannelEvent::Type)state.GetValue(), this));
  }
}

void nsIpcChannel::ReceiveData(nsArrayPtr<const nsUInt8> data)
{
  NS_LOCK(m_ReceiveCallbackMutex);

  if (!m_ReceiveCallback.IsValid())
  {
    m_MessageAccumulator.PushBackRange(data);
    return;
  }

  nsArrayPtr<const nsUInt8> remainingData = data;
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
        nsUInt32 uiRemainingHeaderData = HEADER_SIZE - m_MessageAccumulator.GetCount();
        nsArrayPtr<const nsUInt8> headerData = remainingData.GetSubArray(0, uiRemainingHeaderData);
        m_MessageAccumulator.PushBackRange(headerData);
        NS_ASSERT_DEBUG(m_MessageAccumulator.GetCount() == HEADER_SIZE, "We should have a full header now.");
        remainingData = remainingData.GetSubArray(uiRemainingHeaderData);
      }
    }

    NS_ASSERT_DEBUG(m_MessageAccumulator.GetCount() >= HEADER_SIZE, "Header must be complete at this point.");
    if (remainingData.IsEmpty())
      return;

    // Read and verify header
    nsUInt32 uiMagic = *reinterpret_cast<const nsUInt32*>(m_MessageAccumulator.GetData());
    NS_IGNORE_UNUSED(uiMagic);
    NS_ASSERT_DEBUG(uiMagic == MAGIC_VALUE, "Message received with wrong magic value.");
    nsUInt32 uiMessageSize = *reinterpret_cast<const nsUInt32*>(m_MessageAccumulator.GetData() + 4);
    NS_ASSERT_DEBUG(uiMessageSize < MAX_MESSAGE_SIZE, "Message too big: {0}! Either the stream got corrupted or you need to increase MAX_MESSAGE_SIZE.", uiMessageSize);
    if (uiMessageSize > remainingData.GetCount() + m_MessageAccumulator.GetCount())
    {
      m_MessageAccumulator.PushBackRange(remainingData);
      return;
    }

    // Write missing data into message accumulator
    nsUInt32 remainingMessageData = uiMessageSize - m_MessageAccumulator.GetCount();
    nsArrayPtr<const nsUInt8> messageData = remainingData.GetSubArray(0, remainingMessageData);
    m_MessageAccumulator.PushBackRange(messageData);
    NS_ASSERT_DEBUG(m_MessageAccumulator.GetCount() == uiMessageSize, "");
    remainingData = remainingData.GetSubArray(remainingMessageData);

    {
      m_ReceiveCallback(nsArrayPtr<const nsUInt8>(m_MessageAccumulator.GetData() + HEADER_SIZE, uiMessageSize - HEADER_SIZE));
      m_IncomingMessages.RaiseSignal();
      m_Events.Broadcast(nsIpcChannelEvent(nsIpcChannelEvent::NewMessages, this));
      m_MessageAccumulator.Clear();
    }
  }
}

void nsIpcChannel::FlushPendingOperations()
{
  m_pOwner->WaitForMessages(-1, this);
}
