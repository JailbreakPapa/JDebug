#pragma once

#include <Core/World/World.h>
#include <Foundation/Communication/Message.h>

/// \brief Base class for all messages that are sent as 'events'
struct NS_CORE_DLL nsEventMessage : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsEventMessage, nsMessage);

  nsGameObjectHandle m_hSenderObject;
  nsComponentHandle m_hSenderComponent;

  NS_ALWAYS_INLINE void FillFromSenderComponent(const nsComponent* pSenderComponent)
  {
    if (pSenderComponent != nullptr)
    {
      m_hSenderComponent = pSenderComponent->GetHandle();
      m_hSenderObject = pSenderComponent->GetOwner()->GetHandle();
    }
  }
};

namespace nsInternal
{
  struct NS_CORE_DLL EventMessageSenderHelper
  {
    static bool SendEventMessage(nsMessage& ref_msg, nsComponent* pSenderComponent, nsGameObject* pSearchObject, nsSmallArray<nsComponentHandle, 1>& inout_cachedReceivers);
    static bool SendEventMessage(nsMessage& ref_msg, const nsComponent* pSenderComponent, const nsGameObject* pSearchObject, nsSmallArray<nsComponentHandle, 1>& inout_cachedReceivers);
    static void PostEventMessage(const nsMessage& msg, const nsComponent* pSenderComponent, const nsGameObject* pSearchObject, nsSmallArray<nsComponentHandle, 1>& inout_cachedReceivers, nsTime delay, nsObjectMsgQueueType::Enum queueType);
  };
} // namespace nsInternal

/// \brief A message sender that sends all messages to the next component derived from nsEventMessageHandlerComponent
///   up in the hierarchy starting with the given search object. If none is found the message is sent to
///   all components registered as global event message handler. The receiver is cached after the first send/post call.
template <typename EventMessageType>
class nsEventMessageSender : public nsMessageSenderBase<EventMessageType>
{
public:
  NS_ALWAYS_INLINE bool SendEventMessage(EventMessageType& inout_msg, nsComponent* pSenderComponent, nsGameObject* pSearchObject)
  {
    if constexpr (NS_IS_DERIVED_FROM_STATIC(nsEventMessage, EventMessageType))
    {
      inout_msg.FillFromSenderComponent(pSenderComponent);
    }
    return nsInternal::EventMessageSenderHelper::SendEventMessage(inout_msg, pSenderComponent, pSearchObject, m_CachedReceivers);
  }

  NS_ALWAYS_INLINE bool SendEventMessage(EventMessageType& inout_msg, const nsComponent* pSenderComponent, const nsGameObject* pSearchObject) const
  {
    if constexpr (NS_IS_DERIVED_FROM_STATIC(nsEventMessage, EventMessageType))
    {
      inout_msg.FillFromSenderComponent(pSenderComponent);
    }
    return nsInternal::EventMessageSenderHelper::SendEventMessage(inout_msg, pSenderComponent, pSearchObject, m_CachedReceivers);
  }

  NS_ALWAYS_INLINE void PostEventMessage(EventMessageType& ref_msg, nsComponent* pSenderComponent, nsGameObject* pSearchObject,
    nsTime delay, nsObjectMsgQueueType::Enum queueType = nsObjectMsgQueueType::NextFrame)
  {
    if constexpr (NS_IS_DERIVED_FROM_STATIC(nsEventMessage, EventMessageType))
    {
      ref_msg.FillFromSenderComponent(pSenderComponent);
    }
    nsInternal::EventMessageSenderHelper::PostEventMessage(ref_msg, pSenderComponent, pSearchObject, m_CachedReceivers, delay, queueType);
  }

  NS_ALWAYS_INLINE void PostEventMessage(EventMessageType& ref_msg, const nsComponent* pSenderComponent, const nsGameObject* pSearchObject,
    nsTime delay, nsObjectMsgQueueType::Enum queueType = nsObjectMsgQueueType::NextFrame) const
  {
    if constexpr (NS_IS_DERIVED_FROM_STATIC(nsEventMessage, EventMessageType))
    {
      ref_msg.FillFromSenderComponent(pSenderComponent);
    }
    nsInternal::EventMessageSenderHelper::PostEventMessage(ref_msg, pSenderComponent, pSearchObject, m_CachedReceivers, delay, queueType);
  }

  NS_ALWAYS_INLINE void Invalidate()
  {
    m_CachedReceivers.Clear();
    m_CachedReceivers.GetUserData<nsUInt32>() = 0;
  }

private:
  mutable nsSmallArray<nsComponentHandle, 1> m_CachedReceivers;
};
