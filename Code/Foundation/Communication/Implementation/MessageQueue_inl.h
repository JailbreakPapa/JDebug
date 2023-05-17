
template <typename MetaDataType>
wdMessageQueueBase<MetaDataType>::wdMessageQueueBase(wdAllocatorBase* pAllocator)
  : m_Queue(pAllocator)
{
}

template <typename MetaDataType>
wdMessageQueueBase<MetaDataType>::wdMessageQueueBase(const wdMessageQueueBase& rhs, wdAllocatorBase* pAllocator)
  : m_Queue(pAllocator)
{
  m_Queue = rhs.m_Queue;
}

template <typename MetaDataType>
wdMessageQueueBase<MetaDataType>::~wdMessageQueueBase()
{
  Clear();
}

template <typename MetaDataType>
void wdMessageQueueBase<MetaDataType>::operator=(const wdMessageQueueBase& rhs)
{
  m_Queue = rhs.m_Queue;
}

template <typename MetaDataType>
WD_ALWAYS_INLINE typename wdMessageQueueBase<MetaDataType>::Entry& wdMessageQueueBase<MetaDataType>::operator[](wdUInt32 uiIndex)
{
  return m_Queue[uiIndex];
}

template <typename MetaDataType>
WD_ALWAYS_INLINE const typename wdMessageQueueBase<MetaDataType>::Entry& wdMessageQueueBase<MetaDataType>::operator[](wdUInt32 uiIndex) const
{
  return m_Queue[uiIndex];
}

template <typename MetaDataType>
WD_ALWAYS_INLINE wdUInt32 wdMessageQueueBase<MetaDataType>::GetCount() const
{
  return m_Queue.GetCount();
}

template <typename MetaDataType>
WD_ALWAYS_INLINE bool wdMessageQueueBase<MetaDataType>::IsEmpty() const
{
  return m_Queue.IsEmpty();
}

template <typename MetaDataType>
void wdMessageQueueBase<MetaDataType>::Clear()
{
  m_Queue.Clear();
}

template <typename MetaDataType>
WD_ALWAYS_INLINE void wdMessageQueueBase<MetaDataType>::Reserve(wdUInt32 uiCount)
{
  m_Queue.Reserve(uiCount);
}

template <typename MetaDataType>
WD_ALWAYS_INLINE void wdMessageQueueBase<MetaDataType>::Compact()
{
  m_Queue.Compact();
}

template <typename MetaDataType>
void wdMessageQueueBase<MetaDataType>::Enqueue(wdMessage* pMessage, const MetaDataType& metaData)
{
  Entry entry;
  entry.m_pMessage = pMessage;
  entry.m_MetaData = metaData;

  {
    WD_LOCK(m_Mutex);

    m_Queue.PushBack(entry);
  }
}

template <typename MetaDataType>
bool wdMessageQueueBase<MetaDataType>::TryDequeue(wdMessage*& out_pMessage, MetaDataType& out_metaData)
{
  WD_LOCK(m_Mutex);

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
bool wdMessageQueueBase<MetaDataType>::TryPeek(wdMessage*& out_pMessage, MetaDataType& out_metaData)
{
  WD_LOCK(m_Mutex);

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
WD_ALWAYS_INLINE typename wdMessageQueueBase<MetaDataType>::Entry& wdMessageQueueBase<MetaDataType>::Peek()
{
  return m_Queue.PeekFront();
}

template <typename MetaDataType>
WD_ALWAYS_INLINE void wdMessageQueueBase<MetaDataType>::Dequeue()
{
  m_Queue.PopFront();
}

template <typename MetaDataType>
template <typename Comparer>
WD_ALWAYS_INLINE void wdMessageQueueBase<MetaDataType>::Sort(const Comparer& comparer)
{
  m_Queue.Sort(comparer);
}

template <typename MetaDataType>
void wdMessageQueueBase<MetaDataType>::Lock()
{
  m_Mutex.Lock();
}

template <typename MetaDataType>
void wdMessageQueueBase<MetaDataType>::Unlock()
{
  m_Mutex.Unlock();
}


template <typename MD, typename A>
wdMessageQueue<MD, A>::wdMessageQueue()
  : wdMessageQueueBase<MD>(A::GetAllocator())
{
}

template <typename MD, typename A>
wdMessageQueue<MD, A>::wdMessageQueue(wdAllocatorBase* pQueueAllocator)
  : wdMessageQueueBase<MD>(pQueueAllocator)
{
}

template <typename MD, typename A>
wdMessageQueue<MD, A>::wdMessageQueue(const wdMessageQueue<MD, A>& rhs)
  : wdMessageQueueBase<MD>(rhs, A::GetAllocator())
{
}

template <typename MD, typename A>
wdMessageQueue<MD, A>::wdMessageQueue(const wdMessageQueueBase<MD>& rhs)
  : wdMessageQueueBase<MD>(rhs, A::GetAllocator())
{
}

template <typename MD, typename A>
void wdMessageQueue<MD, A>::operator=(const wdMessageQueue<MD, A>& rhs)
{
  wdMessageQueueBase<MD>::operator=(rhs);
}

template <typename MD, typename A>
void wdMessageQueue<MD, A>::operator=(const wdMessageQueueBase<MD>& rhs)
{
  wdMessageQueueBase<MD>::operator=(rhs);
}
