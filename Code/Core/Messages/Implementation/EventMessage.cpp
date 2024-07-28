#include <Core/CorePCH.h>

#include <Core/Messages/EventMessage.h>
#include <Core/World/EventMessageHandlerComponent.h>
#include <Core/World/World.h>

// clang-format off
NS_IMPLEMENT_MESSAGE_TYPE(nsEventMessage);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEventMessage, 1, nsRTTIDefaultAllocator<nsEventMessage>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

NS_CHECK_AT_COMPILETIME(sizeof(nsEventMessageSender<nsEventMessage>) == 16);

namespace nsInternal
{
  template <typename World, typename GameObject>
  static void UpdateCachedReceivers(const nsMessage& msg, World& ref_world, GameObject pSearchObject, nsSmallArray<nsComponentHandle, 1>& inout_cachedReceivers)
  {
    if (inout_cachedReceivers.GetUserData<nsUInt32>() == 0)
    {
      using ComponentType = typename std::conditional<std::is_const<World>::value, const nsComponent*, nsComponent*>::type;

      nsHybridArray<ComponentType, 4> eventMsgHandlers;
      ref_world.FindEventMsgHandlers(msg, pSearchObject, eventMsgHandlers);

      for (auto pEventMsgHandler : eventMsgHandlers)
      {
        inout_cachedReceivers.PushBack(pEventMsgHandler->GetHandle());
      }

      inout_cachedReceivers.GetUserData<nsUInt32>() = 1;
    }
  }

  bool EventMessageSenderHelper::SendEventMessage(nsMessage& ref_msg, nsComponent* pSenderComponent, nsGameObject* pSearchObject, nsSmallArray<nsComponentHandle, 1>& inout_cachedReceivers)
  {
    nsWorld* pWorld = pSenderComponent->GetWorld();
    UpdateCachedReceivers(ref_msg, *pWorld, pSearchObject, inout_cachedReceivers);

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    bool bHandlerFound = false;
#endif

    bool bResult = false;
    for (auto hReceiver : inout_cachedReceivers)
    {
      nsComponent* pReceiverComponent = nullptr;
      if (pWorld->TryGetComponent(hReceiver, pReceiverComponent))
      {
        bResult |= pReceiverComponent->SendMessage(ref_msg);
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
        bHandlerFound = true;
#endif
      }
    }

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (!bHandlerFound && ref_msg.GetDebugMessageRouting())
    {
      nsLog::Warning("nsEventMessageSender::SendMessage: No event message handler found for message of type {0}.", ref_msg.GetId());
    }
#endif

    return bResult;
  }

  bool EventMessageSenderHelper::SendEventMessage(nsMessage& ref_msg, const nsComponent* pSenderComponent, const nsGameObject* pSearchObject, nsSmallArray<nsComponentHandle, 1>& inout_cachedReceivers)
  {
    const nsWorld* pWorld = pSenderComponent->GetWorld();
    UpdateCachedReceivers(ref_msg, *pWorld, pSearchObject, inout_cachedReceivers);

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    bool bHandlerFound = false;
#endif

    bool bResult = false;
    for (auto hReceiver : inout_cachedReceivers)
    {
      const nsComponent* pReceiverComponent = nullptr;
      if (pWorld->TryGetComponent(hReceiver, pReceiverComponent))
      {
        bResult |= pReceiverComponent->SendMessage(ref_msg);
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
        bHandlerFound = true;
#endif
      }
    }

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (!bHandlerFound && ref_msg.GetDebugMessageRouting())
    {
      nsLog::Warning("nsEventMessageSender::SendMessage: No event message handler found for message of type {0}.", ref_msg.GetId());
    }
#endif

    return bResult;
  }

  void EventMessageSenderHelper::PostEventMessage(const nsMessage& msg, const nsComponent* pSenderComponent, const nsGameObject* pSearchObject, nsSmallArray<nsComponentHandle, 1>& inout_cachedReceivers, nsTime delay, nsObjectMsgQueueType::Enum queueType)
  {
    const nsWorld* pWorld = pSenderComponent->GetWorld();
    UpdateCachedReceivers(msg, *pWorld, pSearchObject, inout_cachedReceivers);

    if (!inout_cachedReceivers.IsEmpty())
    {
      for (auto hReceiver : inout_cachedReceivers)
      {
        pWorld->PostMessage(hReceiver, msg, delay, queueType);
      }
    }
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    else if (msg.GetDebugMessageRouting())
    {
      nsLog::Warning("nsEventMessageSender::PostMessage: No event message handler found for message of type {0}.", msg.GetId());
    }
#endif
  }

} // namespace nsInternal

NS_STATICLINK_FILE(Core, Core_Messages_Implementation_EventMessage);
