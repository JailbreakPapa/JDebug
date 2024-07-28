
template <typename T>
NS_ALWAYS_INLINE const T& nsRenderDataBatch::Iterator<T>::operator*() const
{
  return *nsStaticCast<const T*>(m_pCurrent->m_pRenderData);
}

template <typename T>
NS_ALWAYS_INLINE const T* nsRenderDataBatch::Iterator<T>::operator->() const
{
  return nsStaticCast<const T*>(m_pCurrent->m_pRenderData);
}

template <typename T>
NS_ALWAYS_INLINE nsRenderDataBatch::Iterator<T>::operator const T*() const
{
  return nsStaticCast<const T*>(m_pCurrent->m_pRenderData);
}

template <typename T>
NS_FORCE_INLINE void nsRenderDataBatch::Iterator<T>::Next()
{
  ++m_pCurrent;

  if (m_Filter.IsValid())
  {
    while (m_pCurrent < m_pEnd && m_Filter(m_pCurrent->m_pRenderData))
    {
      ++m_pCurrent;
    }
  }
}

template <typename T>
NS_ALWAYS_INLINE bool nsRenderDataBatch::Iterator<T>::IsValid() const
{
  return m_pCurrent < m_pEnd;
}

template <typename T>
NS_ALWAYS_INLINE void nsRenderDataBatch::Iterator<T>::operator++()
{
  Next();
}

template <typename T>
NS_FORCE_INLINE nsRenderDataBatch::Iterator<T>::Iterator(const SortableRenderData* pStart, const SortableRenderData* pEnd, Filter filter)
  : m_Filter(filter)
{
  const SortableRenderData* pCurrent = pStart;
  if (m_Filter.IsValid())
  {
    while (pCurrent < pEnd && m_Filter(pCurrent->m_pRenderData))
    {
      ++pCurrent;
    }
  }

  m_pCurrent = pCurrent;
  m_pEnd = pEnd;
}


NS_ALWAYS_INLINE nsUInt32 nsRenderDataBatch::GetCount() const
{
  return m_Data.GetCount();
}

template <typename T>
NS_FORCE_INLINE const T* nsRenderDataBatch::GetFirstData() const
{
  auto it = Iterator<T>(m_Data.GetPtr(), m_Data.GetPtr() + m_Data.GetCount(), m_Filter);
  return it.IsValid() ? (const T*)it : nullptr;
}

template <typename T>
NS_FORCE_INLINE nsRenderDataBatch::Iterator<T> nsRenderDataBatch::GetIterator(nsUInt32 uiStartIndex, nsUInt32 uiCount) const
{
  nsUInt32 uiEndIndex = nsMath::Min(uiStartIndex + uiCount, m_Data.GetCount());
  return Iterator<T>(m_Data.GetPtr() + uiStartIndex, m_Data.GetPtr() + uiEndIndex, m_Filter);
}

//////////////////////////////////////////////////////////////////////////

NS_ALWAYS_INLINE nsUInt32 nsRenderDataBatchList::GetBatchCount() const
{
  return m_Batches.GetCount();
}

NS_FORCE_INLINE nsRenderDataBatch nsRenderDataBatchList::GetBatch(nsUInt32 uiIndex) const
{
  nsRenderDataBatch batch = m_Batches[uiIndex];
  batch.m_Filter = m_Filter;

  return batch;
}
