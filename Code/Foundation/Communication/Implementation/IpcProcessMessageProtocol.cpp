#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/IpcProcessMessageProtocol.h>
// #include <Foundation/Communication/Implementation/MessageLoop.h>
#include <Foundation/Communication/IpcChannel.h>
// #include <Foundation/Communication/RemoteMessage.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Serialization/ReflectionSerializer.h>

nsIpcProcessMessageProtocol::nsIpcProcessMessageProtocol(nsIpcChannel* pChannel)
{
  m_pChannel = pChannel;
  m_pChannel->SetReceiveCallback(nsMakeDelegate(&nsIpcProcessMessageProtocol::ReceiveMessageData, this));
}

nsIpcProcessMessageProtocol::~nsIpcProcessMessageProtocol()
{
  m_pChannel->SetReceiveCallback({});

  nsDeque<nsUniquePtr<nsProcessMessage>> messages;
  SwapWorkQueue(messages);
  messages.Clear();
}

bool nsIpcProcessMessageProtocol::Send(nsProcessMessage* pMsg)
{
  nsContiguousMemoryStreamStorage storage;
  nsMemoryStreamWriter writer(&storage);
  nsReflectionSerializer::WriteObjectToBinary(writer, pMsg->GetDynamicRTTI(), pMsg);
  return m_pChannel->Send(nsArrayPtr<const nsUInt8>(storage.GetData(), storage.GetStorageSize32()));
}

bool nsIpcProcessMessageProtocol::ProcessMessages()
{
  nsDeque<nsUniquePtr<nsProcessMessage>> messages;
  SwapWorkQueue(messages);
  if (messages.IsEmpty())
  {
    return false;
  }

  while (!messages.IsEmpty())
  {
    nsUniquePtr<nsProcessMessage> msg = std::move(messages.PeekFront());
    messages.PopFront();
    m_MessageEvent.Broadcast(msg.Borrow());
  }

  return true;
}

nsResult nsIpcProcessMessageProtocol::WaitForMessages(nsTime timeout)
{
  nsResult res = m_pChannel->WaitForMessages(timeout);
  if (res.Succeeded())
  {
    ProcessMessages();
  }
  return res;
}

void nsIpcProcessMessageProtocol::ReceiveMessageData(nsArrayPtr<const nsUInt8> data)
{
  // Message complete, de-serialize
  nsRawMemoryStreamReader reader(data.GetPtr(), data.GetCount());
  const nsRTTI* pRtti = nullptr;

  nsProcessMessage* pMsg = (nsProcessMessage*)nsReflectionSerializer::ReadObjectFromBinary(reader, pRtti);
  nsUniquePtr<nsProcessMessage> msg(pMsg, nsFoundation::GetDefaultAllocator());
  if (msg != nullptr)
  {
    EnqueueMessage(std::move(msg));
  }
  else
  {
    nsLog::Error("Channel received invalid Message!");
  }
}

void nsIpcProcessMessageProtocol::EnqueueMessage(nsUniquePtr<nsProcessMessage>&& msg)
{
  NS_LOCK(m_IncomingQueueMutex);
  m_IncomingQueue.PushBack(std::move(msg));
}

void nsIpcProcessMessageProtocol::SwapWorkQueue(nsDeque<nsUniquePtr<nsProcessMessage>>& messages)
{
  NS_ASSERT_DEBUG(messages.IsEmpty(), "Swap target must be empty!");
  NS_LOCK(m_IncomingQueueMutex);
  if (m_IncomingQueue.IsEmpty())
    return;
  messages.Swap(m_IncomingQueue);
}
