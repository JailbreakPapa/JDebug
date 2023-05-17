#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>
#include <Foundation/Types/Delegate.h>

/// \brief Identifies an event subscription. Zero is always an invalid subscription ID.
typedef wdUInt32 wdEventSubscriptionID;

/// \brief Specifies the type of wdEvent implementation to use
enum class wdEventType
{
  Default,        /// Default implementation. Does not support modifying the event while broadcasting.
  CopyOnBroadcast /// CopyOnBroadcast implementation. Supports modifying the event while broadcasting.
};

/// \brief This class propagates event information to registered event handlers.
///
/// An event can be anything that "happens" that might be of interest to other code, such
/// that it can react on it in some way.
/// Just create an instance of wdEvent and call Broadcast() on it. Other interested code needs const access to
/// the event variable to be able to call AddEventHandler() and RemoveEventHandler().
/// To pass information to the handlers, create a custom struct with event information
/// and then pass a (const) reference to that data through Broadcast().
///
/// If you need to modify the event while broadcasting, for example inside one of the registered event handlers,
/// set EventType = wdEventType::CopyOnBroadcast. Each broadcast will then copy the event handler array before signaling them, allowing
/// modifications during broadcasting.
///
/// \note A class holding an wdEvent member needs to provide public access to the member for external code to
/// be able to register as an event handler. To make it possible to prevent external code from also raising events,
/// all functions that are needed for listening are const, and all others are non-const.
/// Therefore, simply make event members private and provide const reference access through a public getter.
template <typename EventData, typename MutexType, wdEventType EventType>
class wdEventBase
{
protected:
  /// \brief Constructor.
  wdEventBase(wdAllocatorBase* pAllocator);
  ~wdEventBase();

public:
  /// \brief Notification callback type for events.
  using Handler = wdDelegate<void(EventData)>;

  /// \brief An object that can be passed to wdEvent::AddEventHandler to store the subscription information
  /// and automatically remove the event handler upon destruction.
  class Unsubscriber
  {
    WD_DISALLOW_COPY_AND_ASSIGN(Unsubscriber);

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

    /// \brief If the unsubscriber holds a valid subscription, it will be removed from the target wdEvent.
    void Unsubscribe()
    {
      if (m_SubscriptionID == 0)
        return;

      m_pEvent->RemoveEventHandler(m_SubscriptionID);
      Clear();
    }

    /// \brief Checks whether this unsubscriber has a valid subscription.
    bool IsSubscribed() const { return m_SubscriptionID != 0; }

    /// \brief Resets the unsubscriber. Use when the target wdEvent may have been destroyed and automatic unsubscription cannot be executed
    /// anymore.
    void Clear()
    {
      m_pEvent = nullptr;
      m_SubscriptionID = 0;
    }

  private:
    friend class wdEventBase<EventData, MutexType, EventType>;

    const wdEventBase<EventData, MutexType, EventType>* m_pEvent = nullptr;
    wdEventSubscriptionID m_SubscriptionID = 0;
  };

  /// \brief Implementation specific constants.
  enum
  {
    /// Whether the uiMaxRecursionDepth parameter to Broadcast() is supported in this implementation or not.
    RecursionDepthSupported = (EventType == wdEventType::Default || wdConversionTest<MutexType, wdNoMutex>::sameType == 1) ? 1 : 0,

    /// Default value for the maximum recursion depth of Broadcast.
    /// As limiting the recursion depth is not supported when EventType == wdEventType::CopyAndBroadcast and MutexType != wdNoMutex
    /// the default value for that case is the maximum.
    MaxRecursionDepthDefault = RecursionDepthSupported ? 0 : 255
  };

  /// \brief This function will broadcast to all registered users, that this event has just happened.
  ///  Setting uiMaxRecursionDepth will allow you to permit recursions. When broadcasting consider up to what depth
  ///  you want recursions to be permitted. By default no recursion is allowed.
  void Broadcast(EventData pEventData, wdUInt8 uiMaxRecursionDepth = MaxRecursionDepthDefault); // [tested]

  /// \brief Adds a function as an event handler. All handlers will be notified in the order that they were registered.
  ///
  /// The return value can be stored and used to remove the event handler later again.
  wdEventSubscriptionID AddEventHandler(Handler handler) const; // [tested]

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
  void RemoveEventHandler(wdEventSubscriptionID& inout_id) const;

  /// \brief Checks whether an event handler has already been registered.
  bool HasEventHandler(const Handler& handler) const;

  /// \brief Removes all registered event handlers.
  void Clear();

  // it would be a problem if the wdEvent moves in memory, for instance the Unsubscriber's would point to invalid memory
  WD_DISALLOW_COPY_AND_ASSIGN(wdEventBase);

private:
  // Used to detect recursive broadcasts and then throw asserts at you.
  wdUInt8 m_uiRecursionDepth = 0;
  mutable wdEventSubscriptionID m_NextSubscriptionID = 0;

  mutable MutexType m_Mutex;

#if WD_ENABLED(WD_COMPILE_FOR_DEVELOPMENT)
  const void* m_pSelf = nullptr;
#endif

  struct HandlerData
  {
    Handler m_Handler;
    wdEventSubscriptionID m_SubscriptionID;
  };

  /// \brief A dynamic array allows to have zero overhead as long as no event handlers are registered.
  mutable wdDynamicArray<HandlerData> m_EventHandlers;
};

/// \brief Can be used when wdEvent is used without any additional data
struct wdNoEventData
{
};

/// \brief \see wdEventBase
template <typename EventData, typename MutexType = wdNoMutex, typename AllocatorWrapper = wdDefaultAllocatorWrapper, wdEventType EventType = wdEventType::Default>
class wdEvent : public wdEventBase<EventData, MutexType, EventType>
{
public:
  wdEvent();
  wdEvent(wdAllocatorBase* pAllocator);
};

template <typename EventData, typename MutexType = wdNoMutex, typename AllocatorWrapper = wdDefaultAllocatorWrapper>
using wdCopyOnBroadcastEvent = wdEvent<EventData, MutexType, AllocatorWrapper, wdEventType::CopyOnBroadcast>;

#include <Foundation/Communication/Implementation/Event_inl.h>
