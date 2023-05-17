#pragma once

#include <Foundation/Types/ScopeExit.h>

template <typename EventData, typename MutexType, wdEventType EventType>
wdEventBase<EventData, MutexType, EventType>::wdEventBase(wdAllocatorBase* pAllocator)
  : m_EventHandlers(pAllocator)
{
#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  m_pSelf = this;
#endif
}

template <typename EventData, typename MutexType, wdEventType EventType>
wdEventBase<EventData, MutexType, EventType>::~wdEventBase()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  WD_ASSERT_ALWAYS(m_pSelf == this, "The wdEvent was relocated in memory. This is not allowed, as it breaks the Unsubscribers.");
#endif
}

/// A callback can be registered multiple times with different pass-through data (or even with the same,
/// though that is less useful).
template <typename EventData, typename MutexType, wdEventType EventType>
wdEventSubscriptionID wdEventBase<EventData, MutexType, EventType>::AddEventHandler(Handler handler) const
{
  WD_LOCK(m_Mutex);

  if constexpr (std::is_same_v<MutexType, wdNoMutex>)
  {
    if (EventType == wdEventType::Default)
    {
      WD_ASSERT_DEV(m_uiRecursionDepth == 0, "Can't add or remove event handlers while broadcasting (without a mutex). Either enable the use of a mutex on this event, or switch to wdCopyOnBroadcastEvent if this should be allowed. Since this event does not have a mutex, this error can also happen due to multi-threaded access.");
    }
  }

  WD_ASSERT_DEV(!handler.IsComparable() || !HasEventHandler(handler), "The same event handler cannot be added twice");

  auto& item = m_EventHandlers.ExpandAndGetRef();
  item.m_Handler = std::move(handler);
  item.m_SubscriptionID = ++m_NextSubscriptionID;

  return item.m_SubscriptionID;
}

template <typename EventData, typename MutexType, wdEventType EventType>
void wdEventBase<EventData, MutexType, EventType>::AddEventHandler(Handler handler, Unsubscriber& inout_unsubscriber) const
{
  WD_LOCK(m_Mutex);

  if constexpr (std::is_same_v<MutexType, wdNoMutex>)
  {
    if constexpr (EventType == wdEventType::Default)
    {
      WD_ASSERT_DEV(m_uiRecursionDepth == 0, "Can't add or remove event handlers while broadcasting (without a mutex). Either enable the use of a mutex on this event, or switch to wdCopyOnBroadcastEvent if this should be allowed. Since this event does not have a mutex, this error can also happen due to multi-threaded access.");
    }
  }

  inout_unsubscriber.Unsubscribe();
  inout_unsubscriber.m_pEvent = this;
  inout_unsubscriber.m_SubscriptionID = AddEventHandler(std::move(handler));
}


/// Use exactly the same combination of callback/pass-through-data to unregister an event handlers.
/// Otherwise an error occurs.
template <typename EventData, typename MutexType, wdEventType EventType>
void wdEventBase<EventData, MutexType, EventType>::RemoveEventHandler(const Handler& handler) const
{
  WD_ASSERT_DEV(handler.IsComparable(), "Lambdas that capture data cannot be removed via function pointer. Use an wdEventSubscriptionID instead.");

  WD_LOCK(m_Mutex);

  if constexpr (EventType == wdEventType::Default)
  {
    if constexpr (std::is_same_v<MutexType, wdNoMutex>)
    {
      WD_ASSERT_DEV(m_uiRecursionDepth == 0, "Can't add or remove event handlers while broadcasting (without a mutex). Either enable the use of a mutex on this event, or switch to wdCopyOnBroadcastEvent if this should be allowed. Since this event does not have a mutex, this error can also happen due to multi-threaded access.");
    }
  }

  for (wdUInt32 idx = 0; idx < m_EventHandlers.GetCount(); ++idx)
  {
    if (m_EventHandlers[idx].m_Handler.IsEqualIfComparable(handler))
    {
      if constexpr (EventType == wdEventType::Default)
      {
        // if this event does not copy the handlers, and we are currently broadcasting
        // we can't shrink the size of the array, however, we can replace elements (the check above says that we have a mutex, so this is fine)

        if (m_uiRecursionDepth > 0)
        {
          // we just write an invalid handler here, and let the broadcast function clean it up for us
          m_EventHandlers[idx].m_Handler = {};
          m_EventHandlers[idx].m_SubscriptionID = {};
          return;
        }
      }

      // if we are not broadcasting, or the broadcast uses a copy anyway, we can just modify the handler array directly
      m_EventHandlers.RemoveAtAndCopy(idx);
      return;
    }
  }

  WD_ASSERT_DEV(false, "wdEvent::RemoveEventHandler: Handler has not been registered or already been unregistered.");
}

template <typename EventData, typename MutexType, wdEventType EventType>
void wdEventBase<EventData, MutexType, EventType>::RemoveEventHandler(wdEventSubscriptionID& inout_id) const
{
  if (inout_id == 0)
    return;

  const wdEventSubscriptionID subId = inout_id;
  inout_id = 0;

  WD_LOCK(m_Mutex);

  if constexpr (EventType == wdEventType::Default)
  {
    if constexpr (std::is_same_v<MutexType, wdNoMutex>)
    {
      WD_ASSERT_DEV(m_uiRecursionDepth == 0, "Can't add or remove event handlers while broadcasting (without a mutex). Either enable the use of a mutex on this event, or switch to wdCopyOnBroadcastEvent if this should be allowed. Since this event does not have a mutex, this error can also happen due to multi-threaded access.");
    }
  }

  for (wdUInt32 idx = 0; idx < m_EventHandlers.GetCount(); ++idx)
  {
    if (m_EventHandlers[idx].m_SubscriptionID == subId)
    {
      if constexpr (EventType == wdEventType::Default)
      {
        // if this event does not copy the handlers, and we are currently broadcasting
        // we can't shrink the size of the array, however, we can replace elements (the check above says that we have a mutex, so this is fine)

        if (m_uiRecursionDepth > 0)
        {
          // we just write an invalid handler here, and let the broadcast function clean it up for us
          m_EventHandlers[idx].m_Handler = {};
          m_EventHandlers[idx].m_SubscriptionID = {};
          return;
        }
      }

      // if we are not broadcasting, or the broadcast uses a copy anyway, we can just modify the handler array directly
      m_EventHandlers.RemoveAtAndCopy(idx);
      return;
    }
  }

  WD_ASSERT_DEV(false, "wdEvent::RemoveEventHandler: Invalid subscription ID '{0}'.", (wdInt32)subId);
}

template <typename EventData, typename MutexType, wdEventType EventType>
bool wdEventBase<EventData, MutexType, EventType>::HasEventHandler(const Handler& handler) const
{
  WD_ASSERT_DEV(handler.IsComparable(), "Lambdas that capture data cannot be checked via function pointer. Use an wdEventSubscriptionID instead.");

  WD_LOCK(m_Mutex);

  for (wdUInt32 i = 0; i < m_EventHandlers.GetCount(); ++i)
  {
    if (m_EventHandlers[i].m_Handler.IsEqualIfComparable(handler))
      return true;
  }

  return false;
}

template <typename EventData, typename MutexType, wdEventType EventType>
void wdEventBase<EventData, MutexType, EventType>::Clear()
{
  m_EventHandlers.Clear();
}

/// The notification is sent to all event handlers in the order that they were registered.
template <typename EventData, typename MutexType, wdEventType EventType>
void wdEventBase<EventData, MutexType, EventType>::Broadcast(EventData eventData, wdUInt8 uiMaxRecursionDepth)
{
  if constexpr (EventType == wdEventType::Default)
  {
    WD_LOCK(m_Mutex);

    WD_ASSERT_DEV(m_uiRecursionDepth <= uiMaxRecursionDepth, "The event has been triggered recursively or from several threads simultaneously.");

    if (m_uiRecursionDepth > uiMaxRecursionDepth)
      return;

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
    WD_ASSERT_ALWAYS(m_pSelf == this, "The wdEvent was relocated in memory. This is not allowed, as it breaks the Unsubscribers.");
#endif

    m_uiRecursionDepth++;

    // RAII to ensure correctness in case exceptions are used
    auto scopeExit = wdMakeScopeExit([&]() {
      m_uiRecursionDepth--;
    });

    // don't execute handlers that are added while we are broadcasting
    wdUInt32 uiMaxHandlers = m_EventHandlers.GetCount();

    for (wdUInt32 ui = 0; ui < uiMaxHandlers;)
    {
      if (m_EventHandlers[ui].m_Handler.IsValid())
      {
        m_EventHandlers[ui].m_Handler(eventData);
        ++ui;
      }
      else
      {
        m_EventHandlers.RemoveAtAndCopy(ui);
        --uiMaxHandlers;
      }
    }
  }
  else
  {
    wdHybridArray<HandlerData, 16> eventHandlers;
    {
      WD_LOCK(m_Mutex);

      if constexpr (RecursionDepthSupported)
      {
        WD_ASSERT_DEV(m_uiRecursionDepth <= uiMaxRecursionDepth, "The event has been triggered recursively or from several threads simultaneously.");

        if (m_uiRecursionDepth > uiMaxRecursionDepth)
          return;

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
        WD_ASSERT_ALWAYS(m_pSelf == this, "The wdEvent was relocated in memory. This is not allowed, as it breaks the Unsubscribers.");
#endif

        m_uiRecursionDepth++;
      }
      else
      {
        WD_ASSERT_DEV(uiMaxRecursionDepth == 255, "uiMaxRecursionDepth is not supported if wdEventType::CopyOnBroadcast is used and the event needs to be threadsafe.");
      }

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
      WD_ASSERT_ALWAYS(m_pSelf == this, "The wdEvent was relocated in memory. This is not allowed, as it breaks the Unsubscribers.");
#endif

      eventHandlers = m_EventHandlers;
    }

    // RAII to ensure correctness in case exceptions are used
    auto scopeExit = wdMakeScopeExit([&]() {
    // Bug in MSVC 2017. Can't use if constexpr.
#if WD_ENABLED(WD_COMPILER_MSVC) && _MSC_VER < 1920
      if (RecursionDepthSupported)
      {
        m_uiRecursionDepth--;
      }
#else
      if constexpr (RecursionDepthSupported)
      {
        m_uiRecursionDepth--;
      }
#endif
    });

    const wdUInt32 uiHandlerCount = eventHandlers.GetCount();
    for (wdUInt32 ui = 0; ui < uiHandlerCount; ++ui)
    {
      eventHandlers[ui].m_Handler(eventData);
    }
  }
}


template <typename EventData, typename MutexType, typename AllocatorWrapper, wdEventType EventType>
wdEvent<EventData, MutexType, AllocatorWrapper, EventType>::wdEvent()
  : wdEventBase<EventData, MutexType, EventType>(AllocatorWrapper::GetAllocator())
{
}

template <typename EventData, typename MutexType, typename AllocatorWrapper, wdEventType EventType>
wdEvent<EventData, MutexType, AllocatorWrapper, EventType>::wdEvent(wdAllocatorBase* pAllocator)
  : wdEventBase<EventData, MutexType, EventType>(pAllocator)
{
}
