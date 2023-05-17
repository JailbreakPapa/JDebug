
#pragma once

#include <Foundation/Communication/Message.h>
#include <Foundation/Containers/Deque.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

/// \brief Implementation of a message queue on top of a deque.
///
/// Enqueue and TryDequeue/TryPeek methods are thread safe all the others are not. To ensure
/// thread safety for all methods the queue can be locked using wdLock like a mutex.
/// Every entry consists of a pointer to a message and some meta data.
/// Lifetime of the enqueued messages needs to be managed by the user.
/// \see wdMessage
template <typename MetaDataType>
class wdMessageQueueBase
{
public:
  struct Entry
  {
    WD_DECLARE_POD_TYPE();

    wdMessage* m_pMessage;
    MetaDataType m_MetaData;
    mutable wdUInt64 m_uiMessageHash = 0;
  };

protected:
  /// \brief No memory is allocated during construction.
  wdMessageQueueBase(wdAllocatorBase* pAllocator); // [tested]

  /// \brief No memory is allocated during construction.
  wdMessageQueueBase(const wdMessageQueueBase& rhs, wdAllocatorBase* pAllocator);

  /// \brief Destructor.
  ~wdMessageQueueBase(); // [tested]

  /// \brief Assignment operator.
  void operator=(const wdMessageQueueBase& rhs);

public:
  /// \brief Returns the element at the given index. Not thread safe.
  Entry& operator[](wdUInt32 uiIndex); // [tested]

  /// \brief Returns the element at the given index. Not thread safe.
  const Entry& operator[](wdUInt32 uiIndex) const; // [tested]

  /// \brief Returns the number of active elements in the queue.
  wdUInt32 GetCount() const;

  /// \brief Returns true, if the queue does not contain any elements.
  bool IsEmpty() const;

  /// \brief Destructs all elements and sets the count to zero. Does not deallocate any data.
  void Clear();

  /// \brief Expands the queue so it can at least store the given capacity.
  void Reserve(wdUInt32 uiCount);

  /// \brief Tries to compact the array to avoid wasting memory.The resulting capacity is at least 'GetCount' (no elements get removed).
  void Compact();

  /// \brief Enqueues the given message and meta-data. This method is thread safe.
  void Enqueue(wdMessage* pMessage, const MetaDataType& metaData); // [tested]

  /// \brief Dequeues the first element if the queue is not empty and returns true. Returns false if the queue is empty. This method is thread safe.
  bool TryDequeue(wdMessage*& out_pMessage, MetaDataType& out_metaData); // [tested]

  /// \brief Gives the first element if the queue is not empty and returns true. Returns false if the queue is empty. This method is thread safe.
  bool TryPeek(wdMessage*& out_pMessage, MetaDataType& out_metaData); // [tested]

  /// \brief Returns the first element in the queue. Not thread safe.
  Entry& Peek();

  /// \brief Removes the first element from the queue. Not thread safe.
  void Dequeue();

  /// \brief Sort with explicit comparer. Not thread safe.
  template <typename Comparer>
  void Sort(const Comparer& comparer); // [tested]

  /// \brief Acquires an exclusive lock on the queue. Do not use this method directly but use wdLock instead.
  void Lock(); // [tested]

  /// \brief Releases a lock that has been previously acquired. Do not use this method directly but use wdLock instead.
  void Unlock(); // [tested]

private:
  wdDeque<Entry, wdNullAllocatorWrapper> m_Queue;
  wdMutex m_Mutex;
};

/// \brief \see wdMessageQueueBase
template <typename MetaDataType, typename AllocatorWrapper = wdDefaultAllocatorWrapper>
class wdMessageQueue : public wdMessageQueueBase<MetaDataType>
{
public:
  wdMessageQueue();
  wdMessageQueue(wdAllocatorBase* pAllocator);

  wdMessageQueue(const wdMessageQueue<MetaDataType, AllocatorWrapper>& rhs);
  wdMessageQueue(const wdMessageQueueBase<MetaDataType>& rhs);

  void operator=(const wdMessageQueue<MetaDataType, AllocatorWrapper>& rhs);
  void operator=(const wdMessageQueueBase<MetaDataType>& rhs);
};

#include <Foundation/Communication/Implementation/MessageQueue_inl.h>
