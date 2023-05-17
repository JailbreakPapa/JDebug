#pragma once

template <typename KEY, typename VALUE>
inline wdArrayMapBase<KEY, VALUE>::wdArrayMapBase(wdAllocatorBase* pAllocator)
  : m_Data(pAllocator)
{
  m_bSorted = true;
}

template <typename KEY, typename VALUE>
inline wdArrayMapBase<KEY, VALUE>::wdArrayMapBase(const wdArrayMapBase& rhs, wdAllocatorBase* pAllocator)
  : m_bSorted(rhs.m_bSorted)
  , m_Data(pAllocator)
{
  m_Data = rhs.m_Data;
}

template <typename KEY, typename VALUE>
inline void wdArrayMapBase<KEY, VALUE>::operator=(const wdArrayMapBase& rhs)
{
  m_bSorted = rhs.m_bSorted;
  m_Data = rhs.m_Data;
}

template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE wdUInt32 wdArrayMapBase<KEY, VALUE>::GetCount() const
{
  return m_Data.GetCount();
}

template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE bool wdArrayMapBase<KEY, VALUE>::IsEmpty() const
{
  return m_Data.IsEmpty();
}

template <typename KEY, typename VALUE>
inline void wdArrayMapBase<KEY, VALUE>::Clear()
{
  m_bSorted = true;
  m_Data.Clear();
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType, typename CompatibleValueType>
inline wdUInt32 wdArrayMapBase<KEY, VALUE>::Insert(CompatibleKeyType&& key, CompatibleValueType&& value)
{
  Pair& ref = m_Data.ExpandAndGetRef();
  ref.key = std::forward<CompatibleKeyType>(key);
  ref.value = std::forward<CompatibleValueType>(value);
  m_bSorted = false;
  return m_Data.GetCount() - 1;
}

template <typename KEY, typename VALUE>
inline void wdArrayMapBase<KEY, VALUE>::Sort() const
{
  if (m_bSorted)
    return;

  m_bSorted = true;
  m_Data.Sort();
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
wdUInt32 wdArrayMapBase<KEY, VALUE>::Find(const CompatibleKeyType& key) const
{
  if (!m_bSorted)
  {
    m_Data.Sort();
  }

  wdUInt32 lb = 0;
  wdUInt32 ub = m_Data.GetCount();

  while (lb < ub)
  {
    const wdUInt32 middle = lb + ((ub - lb) >> 1);

    if (m_Data[middle].key < key)
    {
      lb = middle + 1;
    }
    else if (key < m_Data[middle].key)
    {
      ub = middle;
    }
    else // equal
    {
      return middle;
    }
  }

  return wdInvalidIndex;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
wdUInt32 wdArrayMapBase<KEY, VALUE>::LowerBound(const CompatibleKeyType& key) const
{
  if (!m_bSorted)
  {
    m_Data.Sort();
  }

  wdUInt32 lb = 0;
  wdUInt32 ub = m_Data.GetCount();

  while (lb < ub)
  {
    const wdUInt32 middle = lb + ((ub - lb) >> 1);

    if (m_Data[middle].key < key)
    {
      lb = middle + 1;
    }
    else
    {
      ub = middle;
    }
  }

  if (lb == m_Data.GetCount())
    return wdInvalidIndex;

  return lb;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
wdUInt32 wdArrayMapBase<KEY, VALUE>::UpperBound(const CompatibleKeyType& key) const
{
  if (!m_bSorted)
  {
    m_Data.Sort();
  }

  wdUInt32 lb = 0;
  wdUInt32 ub = m_Data.GetCount();

  while (lb < ub)
  {
    const wdUInt32 middle = lb + ((ub - lb) >> 1);

    if (key < m_Data[middle].key)
    {
      ub = middle;
    }
    else
    {
      lb = middle + 1;
    }
  }

  if (ub == m_Data.GetCount())
    return wdInvalidIndex;

  return ub;
}

template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE const KEY& wdArrayMapBase<KEY, VALUE>::GetKey(wdUInt32 uiIndex) const
{
  return m_Data[uiIndex].key;
}

template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE const VALUE& wdArrayMapBase<KEY, VALUE>::GetValue(wdUInt32 uiIndex) const
{
  return m_Data[uiIndex].value;
}

template <typename KEY, typename VALUE>
VALUE& wdArrayMapBase<KEY, VALUE>::GetValue(wdUInt32 uiIndex)
{
  return m_Data[uiIndex].value;
}

template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE wdDynamicArray<typename wdArrayMapBase<KEY, VALUE>::Pair>& wdArrayMapBase<KEY, VALUE>::GetData()
{
  m_bSorted = false;
  return m_Data;
}

template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE const wdDynamicArray<typename wdArrayMapBase<KEY, VALUE>::Pair>& wdArrayMapBase<KEY, VALUE>::GetData() const
{
  return m_Data;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
VALUE& wdArrayMapBase<KEY, VALUE>::FindOrAdd(const CompatibleKeyType& key, bool* out_pExisted)
{
  wdUInt32 index = Find<CompatibleKeyType>(key);

  if (out_pExisted)
    *out_pExisted = index != wdInvalidIndex;

  if (index == wdInvalidIndex)
  {
    index = Insert(key, VALUE());
  }

  return GetValue(index);
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
WD_ALWAYS_INLINE VALUE& wdArrayMapBase<KEY, VALUE>::operator[](const CompatibleKeyType& key)
{
  return FindOrAdd(key);
}

template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE const typename wdArrayMapBase<KEY, VALUE>::Pair& wdArrayMapBase<KEY, VALUE>::GetPair(wdUInt32 uiIndex) const
{
  return m_Data[uiIndex];
}

template <typename KEY, typename VALUE>
void wdArrayMapBase<KEY, VALUE>::RemoveAtAndCopy(wdUInt32 uiIndex, bool bKeepSorted)
{
  if (bKeepSorted && m_bSorted)
  {
    m_Data.RemoveAtAndCopy(uiIndex);
  }
  else
  {
    m_Data.RemoveAtAndSwap(uiIndex);
    m_bSorted = false;
  }
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
bool wdArrayMapBase<KEY, VALUE>::RemoveAndCopy(const CompatibleKeyType& key, bool bKeepSorted)
{
  const wdUInt32 uiIndex = Find(key);

  if (uiIndex == wdInvalidIndex)
    return false;

  RemoveAtAndCopy(uiIndex, bKeepSorted);
  return true;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
WD_ALWAYS_INLINE bool wdArrayMapBase<KEY, VALUE>::Contains(const CompatibleKeyType& key) const
{
  return Find(key) != wdInvalidIndex;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
bool wdArrayMapBase<KEY, VALUE>::Contains(const CompatibleKeyType& key, const VALUE& value) const
{
  wdUInt32 atpos = LowerBound(key);

  if (atpos == wdInvalidIndex)
    return false;

  while (atpos < m_Data.GetCount())
  {
    if (m_Data[atpos].key != key)
      return false;

    if (m_Data[atpos].value == value)
      return true;

    ++atpos;
  }

  return false;
}


template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE void wdArrayMapBase<KEY, VALUE>::Reserve(wdUInt32 uiSize)
{
  m_Data.Reserve(uiSize);
}

template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE void wdArrayMapBase<KEY, VALUE>::Compact()
{
  m_Data.Compact();
}

template <typename KEY, typename VALUE>
bool wdArrayMapBase<KEY, VALUE>::operator==(const wdArrayMapBase<KEY, VALUE>& rhs) const
{
  Sort();
  rhs.Sort();

  return m_Data == rhs.m_Data;
}

template <typename KEY, typename VALUE>
WD_ALWAYS_INLINE bool wdArrayMapBase<KEY, VALUE>::operator!=(const wdArrayMapBase<KEY, VALUE>& rhs) const
{
  return !(*this == rhs);
}

template <typename KEY, typename VALUE, typename A>
wdArrayMap<KEY, VALUE, A>::wdArrayMap()
  : wdArrayMapBase<KEY, VALUE>(A::GetAllocator())
{
}

template <typename KEY, typename VALUE, typename A>
wdArrayMap<KEY, VALUE, A>::wdArrayMap(wdAllocatorBase* pAllocator)
  : wdArrayMapBase<KEY, VALUE>(pAllocator)
{
}

template <typename KEY, typename VALUE, typename A>
wdArrayMap<KEY, VALUE, A>::wdArrayMap(const wdArrayMap<KEY, VALUE, A>& rhs)
  : wdArrayMapBase<KEY, VALUE>(rhs, A::GetAllocator())
{
}

template <typename KEY, typename VALUE, typename A>
wdArrayMap<KEY, VALUE, A>::wdArrayMap(const wdArrayMapBase<KEY, VALUE>& rhs)
  : wdArrayMapBase<KEY, VALUE>(rhs, A::GetAllocator())
{
}

template <typename KEY, typename VALUE, typename A>
void wdArrayMap<KEY, VALUE, A>::operator=(const wdArrayMap<KEY, VALUE, A>& rhs)
{
  wdArrayMapBase<KEY, VALUE>::operator=(rhs);
}

template <typename KEY, typename VALUE, typename A>
void wdArrayMap<KEY, VALUE, A>::operator=(const wdArrayMapBase<KEY, VALUE>& rhs)
{
  wdArrayMapBase<KEY, VALUE>::operator=(rhs);
}
