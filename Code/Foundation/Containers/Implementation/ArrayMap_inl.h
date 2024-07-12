#pragma once

template <typename KEY, typename VALUE>
inline nsArrayMapBase<KEY, VALUE>::nsArrayMapBase(nsAllocator* pAllocator)
  : m_Data(pAllocator)
{
  m_bSorted = true;
}

template <typename KEY, typename VALUE>
inline nsArrayMapBase<KEY, VALUE>::nsArrayMapBase(const nsArrayMapBase& rhs, nsAllocator* pAllocator)
  : m_bSorted(rhs.m_bSorted)
  , m_Data(pAllocator)
{
  m_Data = rhs.m_Data;
}

template <typename KEY, typename VALUE>
inline void nsArrayMapBase<KEY, VALUE>::operator=(const nsArrayMapBase& rhs)
{
  m_bSorted = rhs.m_bSorted;
  m_Data = rhs.m_Data;
}

template <typename KEY, typename VALUE>
NS_ALWAYS_INLINE nsUInt32 nsArrayMapBase<KEY, VALUE>::GetCount() const
{
  return m_Data.GetCount();
}

template <typename KEY, typename VALUE>
NS_ALWAYS_INLINE bool nsArrayMapBase<KEY, VALUE>::IsEmpty() const
{
  return m_Data.IsEmpty();
}

template <typename KEY, typename VALUE>
inline void nsArrayMapBase<KEY, VALUE>::Clear()
{
  m_bSorted = true;
  m_Data.Clear();
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType, typename CompatibleValueType>
inline nsUInt32 nsArrayMapBase<KEY, VALUE>::Insert(CompatibleKeyType&& key, CompatibleValueType&& value)
{
  Pair& ref = m_Data.ExpandAndGetRef();
  ref.key = std::forward<CompatibleKeyType>(key);
  ref.value = std::forward<CompatibleValueType>(value);
  m_bSorted = false;
  return m_Data.GetCount() - 1;
}

template <typename KEY, typename VALUE>
inline void nsArrayMapBase<KEY, VALUE>::Sort() const
{
  if (m_bSorted)
    return;

  m_bSorted = true;
  m_Data.Sort();
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
nsUInt32 nsArrayMapBase<KEY, VALUE>::Find(const CompatibleKeyType& key) const
{
  if (!m_bSorted)
  {
    m_Data.Sort();
  }

  nsUInt32 lb = 0;
  nsUInt32 ub = m_Data.GetCount();

  while (lb < ub)
  {
    const nsUInt32 middle = lb + ((ub - lb) >> 1);

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

  return nsInvalidIndex;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
nsUInt32 nsArrayMapBase<KEY, VALUE>::LowerBound(const CompatibleKeyType& key) const
{
  if (!m_bSorted)
  {
    m_Data.Sort();
  }

  nsUInt32 lb = 0;
  nsUInt32 ub = m_Data.GetCount();

  while (lb < ub)
  {
    const nsUInt32 middle = lb + ((ub - lb) >> 1);

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
    return nsInvalidIndex;

  return lb;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
nsUInt32 nsArrayMapBase<KEY, VALUE>::UpperBound(const CompatibleKeyType& key) const
{
  if (!m_bSorted)
  {
    m_Data.Sort();
  }

  nsUInt32 lb = 0;
  nsUInt32 ub = m_Data.GetCount();

  while (lb < ub)
  {
    const nsUInt32 middle = lb + ((ub - lb) >> 1);

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
    return nsInvalidIndex;

  return ub;
}

template <typename KEY, typename VALUE>
NS_ALWAYS_INLINE const KEY& nsArrayMapBase<KEY, VALUE>::GetKey(nsUInt32 uiIndex) const
{
  return m_Data[uiIndex].key;
}

template <typename KEY, typename VALUE>
NS_ALWAYS_INLINE const VALUE& nsArrayMapBase<KEY, VALUE>::GetValue(nsUInt32 uiIndex) const
{
  return m_Data[uiIndex].value;
}

template <typename KEY, typename VALUE>
VALUE& nsArrayMapBase<KEY, VALUE>::GetValue(nsUInt32 uiIndex)
{
  return m_Data[uiIndex].value;
}

template <typename KEY, typename VALUE>
NS_ALWAYS_INLINE nsDynamicArray<typename nsArrayMapBase<KEY, VALUE>::Pair>& nsArrayMapBase<KEY, VALUE>::GetData()
{
  m_bSorted = false;
  return m_Data;
}

template <typename KEY, typename VALUE>
NS_ALWAYS_INLINE const nsDynamicArray<typename nsArrayMapBase<KEY, VALUE>::Pair>& nsArrayMapBase<KEY, VALUE>::GetData() const
{
  return m_Data;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
VALUE& nsArrayMapBase<KEY, VALUE>::FindOrAdd(const CompatibleKeyType& key, bool* out_pExisted)
{
  nsUInt32 index = Find<CompatibleKeyType>(key);

  if (out_pExisted)
    *out_pExisted = index != nsInvalidIndex;

  if (index == nsInvalidIndex)
  {
    index = Insert(key, VALUE());
  }

  return GetValue(index);
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
NS_ALWAYS_INLINE VALUE& nsArrayMapBase<KEY, VALUE>::operator[](const CompatibleKeyType& key)
{
  return FindOrAdd(key);
}

template <typename KEY, typename VALUE>
NS_ALWAYS_INLINE const typename nsArrayMapBase<KEY, VALUE>::Pair& nsArrayMapBase<KEY, VALUE>::GetPair(nsUInt32 uiIndex) const
{
  return m_Data[uiIndex];
}

template <typename KEY, typename VALUE>
void nsArrayMapBase<KEY, VALUE>::RemoveAtAndCopy(nsUInt32 uiIndex, bool bKeepSorted)
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
bool nsArrayMapBase<KEY, VALUE>::RemoveAndCopy(const CompatibleKeyType& key, bool bKeepSorted)
{
  const nsUInt32 uiIndex = Find(key);

  if (uiIndex == nsInvalidIndex)
    return false;

  RemoveAtAndCopy(uiIndex, bKeepSorted);
  return true;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
NS_ALWAYS_INLINE bool nsArrayMapBase<KEY, VALUE>::Contains(const CompatibleKeyType& key) const
{
  return Find(key) != nsInvalidIndex;
}

template <typename KEY, typename VALUE>
template <typename CompatibleKeyType>
bool nsArrayMapBase<KEY, VALUE>::Contains(const CompatibleKeyType& key, const VALUE& value) const
{
  nsUInt32 atpos = LowerBound(key);

  if (atpos == nsInvalidIndex)
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
NS_ALWAYS_INLINE void nsArrayMapBase<KEY, VALUE>::Reserve(nsUInt32 uiSize)
{
  m_Data.Reserve(uiSize);
}

template <typename KEY, typename VALUE>
NS_ALWAYS_INLINE void nsArrayMapBase<KEY, VALUE>::Compact()
{
  m_Data.Compact();
}

template <typename KEY, typename VALUE>
bool nsArrayMapBase<KEY, VALUE>::operator==(const nsArrayMapBase<KEY, VALUE>& rhs) const
{
  Sort();
  rhs.Sort();

  return m_Data == rhs.m_Data;
}

template <typename KEY, typename VALUE, typename A>
nsArrayMap<KEY, VALUE, A>::nsArrayMap()
  : nsArrayMapBase<KEY, VALUE>(A::GetAllocator())
{
}

template <typename KEY, typename VALUE, typename A>
nsArrayMap<KEY, VALUE, A>::nsArrayMap(nsAllocator* pAllocator)
  : nsArrayMapBase<KEY, VALUE>(pAllocator)
{
}

template <typename KEY, typename VALUE, typename A>
nsArrayMap<KEY, VALUE, A>::nsArrayMap(const nsArrayMap<KEY, VALUE, A>& rhs)
  : nsArrayMapBase<KEY, VALUE>(rhs, A::GetAllocator())
{
}

template <typename KEY, typename VALUE, typename A>
nsArrayMap<KEY, VALUE, A>::nsArrayMap(const nsArrayMapBase<KEY, VALUE>& rhs)
  : nsArrayMapBase<KEY, VALUE>(rhs, A::GetAllocator())
{
}

template <typename KEY, typename VALUE, typename A>
void nsArrayMap<KEY, VALUE, A>::operator=(const nsArrayMap<KEY, VALUE, A>& rhs)
{
  nsArrayMapBase<KEY, VALUE>::operator=(rhs);
}

template <typename KEY, typename VALUE, typename A>
void nsArrayMap<KEY, VALUE, A>::operator=(const nsArrayMapBase<KEY, VALUE>& rhs)
{
  nsArrayMapBase<KEY, VALUE>::operator=(rhs);
}
