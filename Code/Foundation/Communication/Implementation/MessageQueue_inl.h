
template <typename MetaDataType>
nsMessageQueueBase<MetaDataType>::nsMessageQueueBase(nsAllocator* pAllocator)
  : m_Queue(pAllocator)
{
}

template <typename MetaDataType>
nsMessageQueueBase<MetaDataType>::nsMessageQueueBase(const nsMessageQueueBase& rhs, nsAllocator* pAllocator)
  : m_Queue(pAllocator)
{
  m_Queue = rhs.m_Queue;
}

template <typename MetaDataType>
nsMessageQueueBase<MetaDataType>::~nsMessageQueueBase()
{
  Clear();
}

template <typename MetaDataType>
void nsMessageQueueBase<MetaDataType>::operator=(const nsMessageQueueBase& rhs)
{
  m_Queue = rhs.m_Queue;
}

template <typename MetaDataType>
NS_ALWAYS_INLINE typename nsMessageQueueBase<MetaDataType>::Entry& nsMessageQueueBase<MetaDataType>::operator[](nsUInt32 uiIndex)
{
  return m_Queue[uiIndex];
}

template <typename MetaDataType>
NS_ALWAYS_INLINE const typename nsMessageQueueBase<MetaDataType>::Entry& nsMessageQueueBase<MetaDataType>::operator[](nsUInt32 uiIndex) const
{
  return m_Queue[uiIndex];
}

template <typename MetaDataType>
NS_ALWAYS_INLINE nsUInt32 nsMessageQueueBase<MetaDataType>::GetCount() const
{
  return m_Queue.GetCount();
}

template <typename MetaDataType>
NS_ALWAYS_INLINE bool nsMessageQueueBase<MetaDataType>::IsEmpty() const
{
  return m_Queue.IsEmpty();
}

template <typename MetaDataType>
void nsMessageQueueBase<MetaDataType>::Clear()
{
  m_Queue.Clear();
}

template <typename MetaDataType>
NS_ALWAYS_INLINE void nsMessageQueueBase<MetaDataType>::Reserve(nsUInt32 uiCount)
{
  m_Queue.Reserve(uiCount);
}

template <typename MetaDataType>
NS_ALWAYS_INLINE void nsMessageQueueBase<MetaDataType>::Compact()
{
  m_Queue.Compact();
}

template <typename MetaDataType>
void nsMessageQueueBase<MetaDataType>::Enqueue(nsMessage* pMessage, const MetaDataType& metaData)
{
  Entry entry;
  entry.m_pMessage = pMessage;
  entry.m_MetaData = metaData;

  {
    NS_LOCK(m_Mutex);

    m_Queue.PushBack(entry);
  }
}

template <typename MetaDataType>
bool nsMessageQueueBase<MetaDataType>::TryDequeue(nsMessage*& out_pMessage, MetaDataType& out_metaData)
{
  NS_LOCK(m_Mutex);

  if (!m_Queue.IsEmpty())
  {
    Entry& entry = m_Queue.PeekFront();
    out_pMessage = entry.m_pMessage;
    out_metaData = entry.m_MetaData;

    m_Queue.PopFront();
    return true;
  }

  return false;
}

template <typename MetaDataType>
bool nsMessageQueueBase<MetaDataType>::TryPeek(nsMessage*& out_pMessage, MetaDataType& out_metaData)
{
  NS_LOCK(m_Mutex);

  if (!m_Queue.IsEmpty())
  {
    Entry& entry = m_Queue.PeekFront();
    out_pMessage = entry.m_pMessage;
    out_metaData = entry.m_MetaData;

    return true;
  }

  return false;
}

template <typename MetaDataType>
NS_ALWAYS_INLINE typename nsMessageQueueBase<MetaDataType>::Entry& nsMessageQueueBase<MetaDataType>::Peek()
{
  return m_Queue.PeekFront();
}

template <typename MetaDataType>
NS_ALWAYS_INLINE void nsMessageQueueBase<MetaDataType>::Dequeue()
{
  m_Queue.PopFront();
}

template <typename MetaDataType>
template <typename Comparer>
NS_ALWAYS_INLINE void nsMessageQueueBase<MetaDataType>::Sort(const Comparer& comparer)
{
  m_Queue.Sort(comparer);
}

template <typename MetaDataType>
void nsMessageQueueBase<MetaDataType>::Lock()
{
  m_Mutex.Lock();
}

template <typename MetaDataType>
void nsMessageQueueBase<MetaDataType>::Unlock()
{
  m_Mutex.Unlock();
}


template <typename MD, typename A>
nsMessageQueue<MD, A>::nsMessageQueue()
  : nsMessageQueueBase<MD>(A::GetAllocator())
{
}

template <typename MD, typename A>
nsMessageQueue<MD, A>::nsMessageQueue(nsAllocator* pQueueAllocator)
  : nsMessageQueueBase<MD>(pQueueAllocator)
{
}

template <typename MD, typename A>
nsMessageQueue<MD, A>::nsMessageQueue(const nsMessageQueue<MD, A>& rhs)
  : nsMessageQueueBase<MD>(rhs, A::GetAllocator())
{
}

template <typename MD, typename A>
nsMessageQueue<MD, A>::nsMessageQueue(const nsMessageQueueBase<MD>& rhs)
  : nsMessageQueueBase<MD>(rhs, A::GetAllocator())
{
}

template <typename MD, typename A>
void nsMessageQueue<MD, A>::operator=(const nsMessageQueue<MD, A>& rhs)
{
  nsMessageQueueBase<MD>::operator=(rhs);
}

template <typename MD, typename A>
void nsMessageQueue<MD, A>::operator=(const nsMessageQueueBase<MD>& rhs)
{
  nsMessageQueueBase<MD>::operator=(rhs);
}
