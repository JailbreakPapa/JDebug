#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Message.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMessage, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

// clang-format on

nsMessageId nsMessage::s_NextMsgId = 0;


void nsMessage::PackageForTransfer(const nsMessage& msg, nsStreamWriter& inout_stream)
{
  const nsRTTI* pRtti = msg.GetDynamicRTTI();

  inout_stream << pRtti->GetTypeNameHash();
  inout_stream << (nsUInt8)pRtti->GetTypeVersion();

  msg.Serialize(inout_stream);
}

nsUniquePtr<nsMessage> nsMessage::ReplicatePackedMessage(nsStreamReader& inout_stream)
{
  nsUInt64 uiTypeHash = 0;
  inout_stream >> uiTypeHash;

  nsUInt8 uiTypeVersion = 0;
  inout_stream >> uiTypeVersion;

  const nsRTTI* pRtti = nsRTTI::FindTypeByNameHash(uiTypeHash);
  if (pRtti == nullptr || !pRtti->GetAllocator()->CanAllocate())
    return nullptr;

  auto pMsg = pRtti->GetAllocator()->Allocate<nsMessage>();

  pMsg->Deserialize(inout_stream, uiTypeVersion);

  return pMsg;
}

NS_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_Message);
