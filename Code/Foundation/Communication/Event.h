#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Types/Delegate.h>

/// \brief Identifies an event subscription. Zero is always an invalid subscription ID.
using nsEventSubscriptionID = nsUInt32;

/// \brief Specifies the type of nsEvent implementation to use
enum class nsEventType
{
  Default,        /// Default implementation. Does not support modifying the event while broadcasting.
  CopyOnBroadcast /// CopyOnBroadcast implementation. Supports modifying the event while broadcasting.
};

/// \brief This class propagates event information to registered event handlers.
///
/// An event can be anything that "happens" that might be of interest to other code, such
/// that it can react on it in some way.
/// Just create an instance of nsEvent and call Broadcast() on it. Other interested code needs const access to
/// the event variable to be able to call AddEventHandler() and RemoveEventHandler().
/// To pass information to the handlers, create a custom struct with event information
/// and then pass a (const) reference to that data through Broadcast().
///
/// If you need to modify the event while broadcasting, for example inside one of the registered event handlers,
/// set EventType = nsEventType::CopyOnBroadcast. Each broadcast will then copy the event handler array before signaling them, allowing
/// modifications during broadcasting.
///
/// \note A class holding an nsEvent member needs to provide public access to the member for external code to
/// be able to register as an event handler. To make it possible to prevent external code from also raising events,
/// all functions that are needed for listening are const, and all others are non-const.
/// Therefore, simply make event members private and provide const reference access through a public getter.
template <typename EventData, typename MutexType, nsEventType EventType>
class nsEventBase
{
protected:
  /// \brief Constructor.
  nsEventBase(nsAllocator* pAllocator);
  ~nsEventBase();

public:
  /// \brief Notification callback type for events.
  using Handler = nsDelegate<void(EventData)>;

  /// \brief An object that can be passed to nsEvent::AddEventHandler to store the subscription information
  /// and automatically remove the event handler upon destruction.
  class Unsubscriber
  {
    NS_DISALLOW_COPY_AND_ASSIGN(Unsubscriber);

  public:
    Unsubscriber() = default;
    Unsubscriber(Unsubscriber&& other)
    {
      m_pEvent = other.m_pEvent;
      m_SubscriptionID = other.m_SubscriptionID;
      other.Clear();
    }
    ~Unsubscriber() { Unsubscribe(); }

    void operator=(Unsubscriber&& other)
    {
      Unsubscribe();

      m_pEvent = other.m_pEvent;
      m_SubscriptionID = other.m_SubscriptionID;
      other.Clear();
    }

    /// \brief If the unsubscriber holds a valid subscription, it will be removed from the target nsEvent.
    void Unsubscribe()
    {
      if (m_SubscriptionID == 0)
        return;

      m_pEvent->RemoveEventHandler(m_SubscriptionID);
      Clear();
    }

    /// \brief Checks whether this unsubscriber has a valid subscription.
    bool IsSubscribed() const { return m_SubscriptionID != 0; }

    /// \brief Resets the unsubscriber. Use when the target nsEvent may have been destroyed and automatic unsubscription cannot be executed
    /// anymore.
    void Clear()
    {
      m_pEvent = nullptr;
      m_SubscriptionID = 0;
    }

  private:
    friend class nsEventBase<EventData, MutexType, EventType>;

    const nsEventBase<EventData, MutexType, EventType>* m_pEvent = nullptr;
    nsEventSubscriptionID m_SubscriptionID = 0;
  };

  /// \brief Implementation specific constants.
  enum
  {
    /// Whether the uiMaxRecursionDepth parameter to Broadcast() is supported in this implementation or not.
    RecursionDepthSupported = (EventType == nsEventType::Default || nsConversionTest<MutexType, nsNoMutex>::sameType == 1) ? 1 : 0,

    /// Default value for the maximum recursion depth of Broadcast.
    /// As limiting the recursion depth is not supported when EventType == nsEventType::CopyAndBroadcast and MutexType != nsNoMutex
    /// the default value for that case is the maximum.
    MaxRecursionDepthDefault = RecursionDepthSupported ? 0 : 255
  };

  /// \brief This function will broadcast to all registered users, that this event has just happened.
  ///  Setting uiMaxRecursionDepth will allow you to permit recursions. When broadcasting consider up to what depth
  ///  you want recursions to be permitted. By default no recursion is allowed.
  void Broadcast(EventData pEventData, nsUInt8 uiMaxRecursionDepth = MaxRecursionDepthDefault); // [tested]

  /// \brief Adds a function as an event handler. All handlers will be notified in the order that they were registered.
  ///
  /// The return value can be stored and used to remove the event handler later again.
  nsEventSubscriptionID AddEventHandler(Handler handler) const; // [tested]

  /// \brief An overload that adds an event handler and initializes the given \a Unsubscriber object.
  ///
  /// When the Unsubscriber is destroyed, it will automatically remove the event handler.
  void AddEventHandler(Handler handler, Unsubscriber& inout_unsubscriber) const; // [tested]

  /// \brief Removes a previously registered handler. It is an error to remove a handler that was not registered.
  void RemoveEventHandler(const Handler& handler) const; // [tested]

  /// \brief Removes a previously registered handler via the returned subscription ID.
  ///
  /// The ID will be reset to zero.
  /// If this is called with a zero ID, nothing happens.
  void RemoveEventHandler(nsEventSubscriptionID& inout_id) const;

  /// \brief Checks whether an event handler has already been registered.
  bool HasEventHandler(const Handler& handler) const;

  /// \brief Removes all registered event handlers.
  void Clear();

  /// \brief Returns true, if no event handlers are registered.
  bool IsEmpty() const;

  // it would be a problem if the nsEvent moves in memory, for instance the Unsubscriber's would point to invalid memory
  NS_DISALLOW_COPY_AND_ASSIGN(nsEventBase);

private:
  // Used to detect recursive broadcasts and then throw asserts at you.
  nsUInt8 m_uiRecursionDepth = 0;
  mutable nsEventSubscriptionID m_NextSubscriptionID = 0;

  mutable MutexType m_Mutex;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  const void* m_pSelf = nullptr;
#endif

  struct HandlerData
  {
    Handler m_Handler;
    nsEventSubscriptionID m_SubscriptionID;
  };

  /// \brief A dynamic array allows to have zero overhead as long as no event handlers are registered.
  mutable nsDynamicArray<HandlerData> m_EventHandlers;
};

/// \brief Can be used when nsEvent is used without any additional data
struct nsNoEventData
{
};

/// \brief \see nsEventBase
template <typename EventData, typename MutexType = nsNoMutex, typename AllocatorWrapper = nsDefaultAllocatorWrapper, nsEventType EventType = nsEventType::Default>
class nsEvent : public nsEventBase<EventData, MutexType, EventType>
{
public:
  nsEvent();
  nsEvent(nsAllocator* pAllocator);
};

template <typename EventData, typename MutexType = nsNoMutex, typename AllocatorWrapper = nsDefaultAllocatorWrapper>
using nsCopyOnBroadcastEvent = nsEvent<EventData, MutexType, AllocatorWrapper, nsEventType::CopyOnBroadcast>;

#include <Foundation/Communication/Implementation/Event_inl.h>
