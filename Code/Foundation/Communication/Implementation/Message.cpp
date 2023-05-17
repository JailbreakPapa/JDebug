#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Message.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdMessage, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

// clang-format on

wdMessageId wdMessage::s_NextMsgId = 0;


void wdMessage::PackageForTransfer(const wdMessage& msg, wdStreamWriter& inout_stream)
{
  const wdRTTI* pRtti = msg.GetDynamicRTTI();

  inout_stream << pRtti->GetTypeNameHash();
  inout_stream << (wdUInt8)pRtti->GetTypeVersion();

  msg.Serialize(inout_stream);
}

wdUniquePtr<wdMessage> wdMessage::ReplicatePackedMessage(wdStreamReader& inout_stream)
{
  wdUInt64 uiTypeHash = 0;
  inout_stream >> uiTypeHash;

  wdUInt8 uiTypeVersion = 0;
  inout_stream >> uiTypeVersion;

  static wdHashTable<wdUInt64, const wdRTTI*, wdHashHelper<wdUInt64>, wdStaticAllocatorWrapper> MessageTypes;

  const wdRTTI* pRtti = nullptr;
  if (!MessageTypes.TryGetValue(uiTypeHash, pRtti))
  {
    for (pRtti = wdRTTI::GetFirstInstance(); pRtti != nullptr; pRtti = pRtti->GetNextInstance())
    {
      if (pRtti->GetTypeNameHash() == uiTypeHash)
      {
        MessageTypes[uiTypeHash] = pRtti;
        break;
      }
    }
  }

  if (pRtti == nullptr || !pRtti->GetAllocator()->CanAllocate())
    return nullptr;

  auto pMsg = pRtti->GetAllocator()->Allocate<wdMessage>();

  pMsg->Deserialize(inout_stream, uiTypeVersion);

  return pMsg;
}

WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_Message);
