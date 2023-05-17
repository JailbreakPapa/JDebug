
/// \brief Value used by containers for indices to indicate an invalid index.
#ifndef wdInvalidIndex
#  define wdInvalidIndex 0xFFFFFFFF
#endif

// ***** Const Iterator *****

template <typename K, typename V, typename H>
wdHashTableBase<K, V, H>::ConstIterator::ConstIterator(const wdHashTableBase<K, V, H>& hashTable)
  : m_pHashTable(&hashTable)
{
}

template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::ConstIterator::SetToBegin()
{
  if (m_pHashTable->IsEmpty())
  {
    m_uiCurrentIndex = m_pHashTable->m_uiCapacity;
    return;
  }
  while (!m_pHashTable->IsValidEntry(m_uiCurrentIndex))
  {
    ++m_uiCurrentIndex;
  }
}

template <typename K, typename V, typename H>
inline void wdHashTableBase<K, V, H>::ConstIterator::SetToEnd()
{
  m_uiCurrentCount = m_pHashTable->m_uiCount;
  m_uiCurrentIndex = m_pHashTable->m_uiCapacity;
}


template <typename K, typename V, typename H>
WD_FORCE_INLINE bool wdHashTableBase<K, V, H>::ConstIterator::IsValid() const
{
  return m_uiCurrentCount < m_pHashTable->m_uiCount;
}

template <typename K, typename V, typename H>
WD_FORCE_INLINE bool wdHashTableBase<K, V, H>::ConstIterator::operator==(const typename wdHashTableBase<K, V, H>::ConstIterator& rhs) const
{
  return m_uiCurrentIndex == rhs.m_uiCurrentIndex && m_pHashTable->m_pEntries == rhs.m_pHashTable->m_pEntries;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE bool wdHashTableBase<K, V, H>::ConstIterator::operator!=(const typename wdHashTableBase<K, V, H>::ConstIterator& rhs) const
{
  return !(*this == rhs);
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE const K& wdHashTableBase<K, V, H>::ConstIterator::Key() const
{
  return m_pHashTable->m_pEntries[m_uiCurrentIndex].key;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE const V& wdHashTableBase<K, V, H>::ConstIterator::Value() const
{
  return m_pHashTable->m_pEntries[m_uiCurrentIndex].value;
}

template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::ConstIterator::Next()
{
  // if we already iterated over the amount of valid elements that the hash-table stores, early out
  if (m_uiCurrentCount >= m_pHashTable->m_uiCount)
    return;

  // increase the counter of how many elements we have seen
  ++m_uiCurrentCount;
  // increase the index of the element to look at
  ++m_uiCurrentIndex;

  // check that we don't leave the valid range of element indices
  while (m_uiCurrentIndex < m_pHashTable->m_uiCapacity)
  {
    if (m_pHashTable->IsValidEntry(m_uiCurrentIndex))
      return;

    ++m_uiCurrentIndex;
  }

  // if we fell through this loop, we reached the end of all elements in the container
  // set the m_uiCurrentCount to maximum, to enable early-out in the future and to make 'IsValid' return 'false'
  m_uiCurrentCount = m_pHashTable->m_uiCount;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE void wdHashTableBase<K, V, H>::ConstIterator::operator++()
{
  Next();
}


// ***** Iterator *****

template <typename K, typename V, typename H>
wdHashTableBase<K, V, H>::Iterator::Iterator(const wdHashTableBase<K, V, H>& hashTable)
  : ConstIterator(hashTable)
{
}

template <typename K, typename V, typename H>
wdHashTableBase<K, V, H>::Iterator::Iterator(const typename wdHashTableBase<K, V, H>::Iterator& rhs)
  : ConstIterator(*rhs.m_pHashTable)
{
  this->m_uiCurrentIndex = rhs.m_uiCurrentIndex;
  this->m_uiCurrentCount = rhs.m_uiCurrentCount;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE void wdHashTableBase<K, V, H>::Iterator::operator=(const Iterator& rhs) // [tested]
{
  this->m_pHashTable = rhs.m_pHashTable;
  this->m_uiCurrentIndex = rhs.m_uiCurrentIndex;
  this->m_uiCurrentCount = rhs.m_uiCurrentCount;
}

template <typename K, typename V, typename H>
WD_FORCE_INLINE V& wdHashTableBase<K, V, H>::Iterator::Value()
{
  return this->m_pHashTable->m_pEntries[this->m_uiCurrentIndex].value;
}


// ***** wdHashTableBase *****

template <typename K, typename V, typename H>
wdHashTableBase<K, V, H>::wdHashTableBase(wdAllocatorBase* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;
}

template <typename K, typename V, typename H>
wdHashTableBase<K, V, H>::wdHashTableBase(const wdHashTableBase<K, V, H>& other, wdAllocatorBase* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;

  *this = other;
}

template <typename K, typename V, typename H>
wdHashTableBase<K, V, H>::wdHashTableBase(wdHashTableBase<K, V, H>&& other, wdAllocatorBase* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;

  *this = std::move(other);
}

template <typename K, typename V, typename H>
wdHashTableBase<K, V, H>::~wdHashTableBase()
{
  Clear();
  WD_DELETE_RAW_BUFFER(m_pAllocator, m_pEntries);
  WD_DELETE_RAW_BUFFER(m_pAllocator, m_pEntryFlags);
  m_uiCapacity = 0;
}

template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::operator=(const wdHashTableBase<K, V, H>& rhs)
{
  Clear();
  Reserve(rhs.GetCount());

  wdUInt32 uiCopied = 0;
  for (wdUInt32 i = 0; uiCopied < rhs.GetCount(); ++i)
  {
    if (rhs.IsValidEntry(i))
    {
      Insert(rhs.m_pEntries[i].key, rhs.m_pEntries[i].value);
      ++uiCopied;
    }
  }
}

template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::operator=(wdHashTableBase<K, V, H>&& rhs)
{
  // Clear any existing data (calls destructors if necessary)
  Clear();

  if (m_pAllocator != rhs.m_pAllocator)
  {
    Reserve(rhs.m_uiCapacity);

    wdUInt32 uiCopied = 0;
    for (wdUInt32 i = 0; uiCopied < rhs.GetCount(); ++i)
    {
      if (rhs.IsValidEntry(i))
      {
        Insert(std::move(rhs.m_pEntries[i].key), std::move(rhs.m_pEntries[i].value));
        ++uiCopied;
      }
    }

    rhs.Clear();
  }
  else
  {
    WD_DELETE_RAW_BUFFER(m_pAllocator, m_pEntries);
    WD_DELETE_RAW_BUFFER(m_pAllocator, m_pEntryFlags);

    // Move all data over.
    m_pEntries = rhs.m_pEntries;
    m_pEntryFlags = rhs.m_pEntryFlags;
    m_uiCount = rhs.m_uiCount;
    m_uiCapacity = rhs.m_uiCapacity;

    // Temp copy forgets all its state.
    rhs.m_pEntries = nullptr;
    rhs.m_pEntryFlags = nullptr;
    rhs.m_uiCount = 0;
    rhs.m_uiCapacity = 0;
  }
}

template <typename K, typename V, typename H>
bool wdHashTableBase<K, V, H>::operator==(const wdHashTableBase<K, V, H>& rhs) const
{
  if (m_uiCount != rhs.m_uiCount)
    return false;

  wdUInt32 uiCompared = 0;
  for (wdUInt32 i = 0; uiCompared < m_uiCount; ++i)
  {
    if (IsValidEntry(i))
    {
      const V* pRhsValue = nullptr;
      if (!rhs.TryGetValue(m_pEntries[i].key, pRhsValue))
        return false;

      if (m_pEntries[i].value != *pRhsValue)
        return false;

      ++uiCompared;
    }
  }

  return true;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE bool wdHashTableBase<K, V, H>::operator!=(const wdHashTableBase<K, V, H>& rhs) const
{
  return !(*this == rhs);
}

template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::Reserve(wdUInt32 uiCapacity)
{
  const wdUInt64 uiCap64 = static_cast<wdUInt64>(uiCapacity);
  wdUInt64 uiNewCapacity64 = uiCap64 + (uiCap64 * 2 / 3); // ensure a maximum load of 60%

  uiNewCapacity64 = wdMath::Min<wdUInt64>(uiNewCapacity64, 0x80000000llu); // the largest power-of-two in 32 bit

  wdUInt32 uiNewCapacity32 = static_cast<wdUInt32>(uiNewCapacity64 & 0xFFFFFFFF);
  WD_ASSERT_DEBUG(uiCapacity <= uiNewCapacity32, "wdHashSet/Map do not support more than 2 billion entries.");

  if (m_uiCapacity >= uiNewCapacity32)
    return;

  uiNewCapacity32 = wdMath::Max<wdUInt32>(wdMath::PowerOfTwo_Ceil(uiNewCapacity32), CAPACITY_ALIGNMENT);
  SetCapacity(uiNewCapacity32);
}

template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::Compact()
{
  if (IsEmpty())
  {
    // completely deallocate all data, if the table is empty.
    WD_DELETE_RAW_BUFFER(m_pAllocator, m_pEntries);
    WD_DELETE_RAW_BUFFER(m_pAllocator, m_pEntryFlags);
    m_uiCapacity = 0;
  }
  else
  {
    const wdUInt32 uiNewCapacity = wdMath::PowerOfTwo_Ceil(m_uiCount + (CAPACITY_ALIGNMENT - 1)) & ~(CAPACITY_ALIGNMENT - 1);
    if (m_uiCapacity != uiNewCapacity)
      SetCapacity(uiNewCapacity);
  }
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE wdUInt32 wdHashTableBase<K, V, H>::GetCount() const
{
  return m_uiCount;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE bool wdHashTableBase<K, V, H>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::Clear()
{
  for (wdUInt32 i = 0; i < m_uiCapacity; ++i)
  {
    if (IsValidEntry(i))
    {
      wdMemoryUtils::Destruct(&m_pEntries[i].key, 1);
      wdMemoryUtils::Destruct(&m_pEntries[i].value, 1);
    }
  }

  wdMemoryUtils::ZeroFill(m_pEntryFlags, GetFlagsCapacity());
  m_uiCount = 0;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType, typename CompatibleValueType>
bool wdHashTableBase<K, V, H>::Insert(CompatibleKeyType&& key, CompatibleValueType&& value, V* out_pOldValue /*= nullptr*/)
{
  Reserve(m_uiCount + 1);

  wdUInt32 uiIndex = H::Hash(key) & (m_uiCapacity - 1);
  wdUInt32 uiDeletedIndex = wdInvalidIndex;

  wdUInt32 uiCounter = 0;
  while (!IsFreeEntry(uiIndex) && uiCounter < m_uiCapacity)
  {
    if (IsDeletedEntry(uiIndex))
    {
      if (uiDeletedIndex == wdInvalidIndex)
        uiDeletedIndex = uiIndex;
    }
    else if (H::Equal(m_pEntries[uiIndex].key, key))
    {
      if (out_pOldValue != nullptr)
        *out_pOldValue = std::move(m_pEntries[uiIndex].value);

      m_pEntries[uiIndex].value = std::forward<CompatibleValueType>(value); // Either move or copy assignment.
      return true;
    }
    ++uiIndex;
    if (uiIndex == m_uiCapacity)
      uiIndex = 0;

    ++uiCounter;
  }

  // new entry
  uiIndex = uiDeletedIndex != wdInvalidIndex ? uiDeletedIndex : uiIndex;

  // Both constructions might either be a move or a copy.
  wdMemoryUtils::CopyOrMoveConstruct(&m_pEntries[uiIndex].key, std::forward<CompatibleKeyType>(key));
  wdMemoryUtils::CopyOrMoveConstruct(&m_pEntries[uiIndex].value, std::forward<CompatibleValueType>(value));

  MarkEntryAsValid(uiIndex);
  ++m_uiCount;

  return false;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
bool wdHashTableBase<K, V, H>::Remove(const CompatibleKeyType& key, V* out_pOldValue /*= nullptr*/)
{
  wdUInt32 uiIndex = FindEntry(key);
  if (uiIndex != wdInvalidIndex)
  {
    if (out_pOldValue != nullptr)
      *out_pOldValue = std::move(m_pEntries[uiIndex].value);

    RemoveInternal(uiIndex);
    return true;
  }

  return false;
}

template <typename K, typename V, typename H>
typename wdHashTableBase<K, V, H>::Iterator wdHashTableBase<K, V, H>::Remove(const typename wdHashTableBase<K, V, H>::Iterator& pos)
{
  WD_ASSERT_DEBUG(pos.m_pHashTable == this, "Iterator from wrong hashtable");
  Iterator it = pos;
  wdUInt32 uiIndex = pos.m_uiCurrentIndex;
  ++it;
  --it.m_uiCurrentCount;
  RemoveInternal(uiIndex);
  return it;
}

template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::RemoveInternal(wdUInt32 uiIndex)
{
  wdMemoryUtils::Destruct(&m_pEntries[uiIndex].key, 1);
  wdMemoryUtils::Destruct(&m_pEntries[uiIndex].value, 1);

  wdUInt32 uiNextIndex = uiIndex + 1;
  if (uiNextIndex == m_uiCapacity)
    uiNextIndex = 0;

  // if the next entry is free we are at the end of a chain and
  // can immediately mark this entry as free as well
  if (IsFreeEntry(uiNextIndex))
  {
    MarkEntryAsFree(uiIndex);

    // run backwards and free all deleted entries in this chain
    wdUInt32 uiPrevIndex = (uiIndex != 0) ? uiIndex : m_uiCapacity;
    --uiPrevIndex;

    while (IsDeletedEntry(uiPrevIndex))
    {
      MarkEntryAsFree(uiPrevIndex);

      if (uiPrevIndex == 0)
        uiPrevIndex = m_uiCapacity;
      --uiPrevIndex;
    }
  }
  else
  {
    MarkEntryAsDeleted(uiIndex);
  }

  --m_uiCount;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline bool wdHashTableBase<K, V, H>::TryGetValue(const CompatibleKeyType& key, V& out_value) const
{
  wdUInt32 uiIndex = FindEntry(key);
  if (uiIndex != wdInvalidIndex)
  {
    out_value = m_pEntries[uiIndex].value;
    return true;
  }

  return false;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline bool wdHashTableBase<K, V, H>::TryGetValue(const CompatibleKeyType& key, const V*& out_pValue) const
{
  wdUInt32 uiIndex = FindEntry(key);
  if (uiIndex != wdInvalidIndex)
  {
    out_pValue = &m_pEntries[uiIndex].value;
    return true;
  }

  return false;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline bool wdHashTableBase<K, V, H>::TryGetValue(const CompatibleKeyType& key, V*& out_pValue) const
{
  wdUInt32 uiIndex = FindEntry(key);
  if (uiIndex != wdInvalidIndex)
  {
    out_pValue = &m_pEntries[uiIndex].value;
    return true;
  }

  return false;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline typename wdHashTableBase<K, V, H>::ConstIterator wdHashTableBase<K, V, H>::Find(const CompatibleKeyType& key) const
{
  wdUInt32 uiIndex = FindEntry(key);
  if (uiIndex == wdInvalidIndex)
  {
    return GetEndIterator();
  }

  ConstIterator it(*this);
  it.m_uiCurrentIndex = uiIndex;
  it.m_uiCurrentCount = 0; // we do not know the 'count' (which is used as an optimization), so we just use 0

  return it;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline typename wdHashTableBase<K, V, H>::Iterator wdHashTableBase<K, V, H>::Find(const CompatibleKeyType& key)
{
  wdUInt32 uiIndex = FindEntry(key);
  if (uiIndex == wdInvalidIndex)
  {
    return GetEndIterator();
  }

  Iterator it(*this);
  it.m_uiCurrentIndex = uiIndex;
  it.m_uiCurrentCount = 0; // we do not know the 'count' (which is used as an optimization), so we just use 0
  return it;
}


template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline const V* wdHashTableBase<K, V, H>::GetValue(const CompatibleKeyType& key) const
{
  wdUInt32 uiIndex = FindEntry(key);
  return (uiIndex != wdInvalidIndex) ? &m_pEntries[uiIndex].value : nullptr;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline V* wdHashTableBase<K, V, H>::GetValue(const CompatibleKeyType& key)
{
  wdUInt32 uiIndex = FindEntry(key);
  return (uiIndex != wdInvalidIndex) ? &m_pEntries[uiIndex].value : nullptr;
}

template <typename K, typename V, typename H>
inline V& wdHashTableBase<K, V, H>::operator[](const K& key)
{
  return FindOrAdd(key, nullptr);
}

template <typename K, typename V, typename H>
V& wdHashTableBase<K, V, H>::FindOrAdd(const K& key, bool* out_pExisted)
{
  const wdUInt32 uiHash = H::Hash(key);
  wdUInt32 uiIndex = FindEntry(uiHash, key);

  if (out_pExisted)
  {
    *out_pExisted = uiIndex != wdInvalidIndex;
  }

  if (uiIndex == wdInvalidIndex)
  {
    Reserve(m_uiCount + 1);

    // search for suitable insertion index again, table might have been resized
    uiIndex = uiHash & (m_uiCapacity - 1);
    while (IsValidEntry(uiIndex))
    {
      ++uiIndex;
      if (uiIndex == m_uiCapacity)
        uiIndex = 0;
    }

    // new entry
    wdMemoryUtils::CopyConstruct(&m_pEntries[uiIndex].key, key, 1);
    wdMemoryUtils::DefaultConstruct(&m_pEntries[uiIndex].value, 1);
    MarkEntryAsValid(uiIndex);
    ++m_uiCount;
  }
  return m_pEntries[uiIndex].value;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
WD_FORCE_INLINE bool wdHashTableBase<K, V, H>::Contains(const CompatibleKeyType& key) const
{
  return FindEntry(key) != wdInvalidIndex;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE typename wdHashTableBase<K, V, H>::Iterator wdHashTableBase<K, V, H>::GetIterator()
{
  Iterator iterator(*this);
  iterator.SetToBegin();
  return iterator;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE typename wdHashTableBase<K, V, H>::Iterator wdHashTableBase<K, V, H>::GetEndIterator()
{
  Iterator iterator(*this);
  iterator.SetToEnd();
  return iterator;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE typename wdHashTableBase<K, V, H>::ConstIterator wdHashTableBase<K, V, H>::GetIterator() const
{
  ConstIterator iterator(*this);
  iterator.SetToBegin();
  return iterator;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE typename wdHashTableBase<K, V, H>::ConstIterator wdHashTableBase<K, V, H>::GetEndIterator() const
{
  ConstIterator iterator(*this);
  iterator.SetToEnd();
  return iterator;
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE wdAllocatorBase* wdHashTableBase<K, V, H>::GetAllocator() const
{
  return m_pAllocator;
}

template <typename K, typename V, typename H>
wdUInt64 wdHashTableBase<K, V, H>::GetHeapMemoryUsage() const
{
  return ((wdUInt64)m_uiCapacity * sizeof(Entry)) + (sizeof(wdUInt32) * (wdUInt64)GetFlagsCapacity());
}

// private methods
template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::SetCapacity(wdUInt32 uiCapacity)
{
  WD_ASSERT_DEV(wdMath::IsPowerOf2(uiCapacity), "uiCapacity must be a power of two to avoid modulo during lookup.");
  const wdUInt32 uiOldCapacity = m_uiCapacity;
  m_uiCapacity = uiCapacity;

  Entry* pOldEntries = m_pEntries;
  wdUInt32* pOldEntryFlags = m_pEntryFlags;

  m_pEntries = WD_NEW_RAW_BUFFER(m_pAllocator, Entry, m_uiCapacity);
  m_pEntryFlags = WD_NEW_RAW_BUFFER(m_pAllocator, wdUInt32, GetFlagsCapacity());
  wdMemoryUtils::ZeroFill(m_pEntryFlags, GetFlagsCapacity());

  m_uiCount = 0;
  for (wdUInt32 i = 0; i < uiOldCapacity; ++i)
  {
    if (GetFlags(pOldEntryFlags, i) == VALID_ENTRY)
    {
      WD_VERIFY(!Insert(std::move(pOldEntries[i].key), std::move(pOldEntries[i].value)), "Implementation error");

      wdMemoryUtils::Destruct(&pOldEntries[i].key, 1);
      wdMemoryUtils::Destruct(&pOldEntries[i].value, 1);
    }
  }

  WD_DELETE_RAW_BUFFER(m_pAllocator, pOldEntries);
  WD_DELETE_RAW_BUFFER(m_pAllocator, pOldEntryFlags);
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
WD_ALWAYS_INLINE wdUInt32 wdHashTableBase<K, V, H>::FindEntry(const CompatibleKeyType& key) const
{
  return FindEntry(H::Hash(key), key);
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline wdUInt32 wdHashTableBase<K, V, H>::FindEntry(wdUInt32 uiHash, const CompatibleKeyType& key) const
{
  if (m_uiCapacity > 0)
  {
    wdUInt32 uiIndex = uiHash & (m_uiCapacity - 1);
    wdUInt32 uiCounter = 0;
    while (!IsFreeEntry(uiIndex) && uiCounter < m_uiCapacity)
    {
      if (IsValidEntry(uiIndex) && H::Equal(m_pEntries[uiIndex].key, key))
        return uiIndex;

      ++uiIndex;
      if (uiIndex == m_uiCapacity)
        uiIndex = 0;

      ++uiCounter;
    }
  }
  // not found
  return wdInvalidIndex;
}

#define WD_HASHTABLE_USE_BITFLAGS WD_ON

template <typename K, typename V, typename H>
WD_FORCE_INLINE wdUInt32 wdHashTableBase<K, V, H>::GetFlagsCapacity() const
{
#if WD_ENABLED(WD_HASHTABLE_USE_BITFLAGS)
  return (m_uiCapacity + 15) / 16;
#else
  return m_uiCapacity;
#endif
}

template <typename K, typename V, typename H>
WD_ALWAYS_INLINE wdUInt32 wdHashTableBase<K, V, H>::GetFlags(wdUInt32* pFlags, wdUInt32 uiEntryIndex) const
{
#if WD_ENABLED(WD_HASHTABLE_USE_BITFLAGS)
  const wdUInt32 uiIndex = uiEntryIndex / 16;
  const wdUInt32 uiSubIndex = (uiEntryIndex & 15) * 2;
  return (pFlags[uiIndex] >> uiSubIndex) & FLAGS_MASK;
#else
  return pFlags[uiEntryIndex] & FLAGS_MASK;
#endif
}

template <typename K, typename V, typename H>
void wdHashTableBase<K, V, H>::SetFlags(wdUInt32 uiEntryIndex, wdUInt32 uiFlags)
{
#if WD_ENABLED(WD_HASHTABLE_USE_BITFLAGS)
  const wdUInt32 uiIndex = uiEntryIndex / 16;
  const wdUInt32 uiSubIndex = (uiEntryIndex & 15) * 2;
  WD_ASSERT_DEBUG(uiIndex < GetFlagsCapacity(), "Out of bounds access");
  m_pEntryFlags[uiIndex] &= ~(FLAGS_MASK << uiSubIndex);
  m_pEntryFlags[uiIndex] |= (uiFlags << uiSubIndex);
#else
  WD_ASSERT_DEV(uiEntryIndex < GetFlagsCapacity(), "Out of bounds access");
  m_pEntryFlags[uiEntryIndex] = uiFlags;
#endif
}

template <typename K, typename V, typename H>
WD_FORCE_INLINE bool wdHashTableBase<K, V, H>::IsFreeEntry(wdUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == FREE_ENTRY;
}

template <typename K, typename V, typename H>
WD_FORCE_INLINE bool wdHashTableBase<K, V, H>::IsValidEntry(wdUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == VALID_ENTRY;
}

template <typename K, typename V, typename H>
WD_FORCE_INLINE bool wdHashTableBase<K, V, H>::IsDeletedEntry(wdUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == DELETED_ENTRY;
}

template <typename K, typename V, typename H>
WD_FORCE_INLINE void wdHashTableBase<K, V, H>::MarkEntryAsFree(wdUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, FREE_ENTRY);
}

template <typename K, typename V, typename H>
WD_FORCE_INLINE void wdHashTableBase<K, V, H>::MarkEntryAsValid(wdUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, VALID_ENTRY);
}

template <typename K, typename V, typename H>
WD_FORCE_INLINE void wdHashTableBase<K, V, H>::MarkEntryAsDeleted(wdUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, DELETED_ENTRY);
}


template <typename K, typename V, typename H, typename A>
wdHashTable<K, V, H, A>::wdHashTable()
  : wdHashTableBase<K, V, H>(A::GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
wdHashTable<K, V, H, A>::wdHashTable(wdAllocatorBase* pAllocator)
  : wdHashTableBase<K, V, H>(pAllocator)
{
}

template <typename K, typename V, typename H, typename A>
wdHashTable<K, V, H, A>::wdHashTable(const wdHashTable<K, V, H, A>& other)
  : wdHashTableBase<K, V, H>(other, A::GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
wdHashTable<K, V, H, A>::wdHashTable(const wdHashTableBase<K, V, H>& other)
  : wdHashTableBase<K, V, H>(other, A::GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
wdHashTable<K, V, H, A>::wdHashTable(wdHashTable<K, V, H, A>&& other)
  : wdHashTableBase<K, V, H>(std::move(other), other.GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
wdHashTable<K, V, H, A>::wdHashTable(wdHashTableBase<K, V, H>&& other)
  : wdHashTableBase<K, V, H>(std::move(other), other.GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
void wdHashTable<K, V, H, A>::operator=(const wdHashTable<K, V, H, A>& rhs)
{
  wdHashTableBase<K, V, H>::operator=(rhs);
}

template <typename K, typename V, typename H, typename A>
void wdHashTable<K, V, H, A>::operator=(const wdHashTableBase<K, V, H>& rhs)
{
  wdHashTableBase<K, V, H>::operator=(rhs);
}

template <typename K, typename V, typename H, typename A>
void wdHashTable<K, V, H, A>::operator=(wdHashTable<K, V, H, A>&& rhs)
{
  wdHashTableBase<K, V, H>::operator=(std::move(rhs));
}

template <typename K, typename V, typename H, typename A>
void wdHashTable<K, V, H, A>::operator=(wdHashTableBase<K, V, H>&& rhs)
{
  wdHashTableBase<K, V, H>::operator=(std::move(rhs));
}

template <typename KeyType, typename ValueType, typename Hasher>
void wdHashTableBase<KeyType, ValueType, Hasher>::Swap(wdHashTableBase<KeyType, ValueType, Hasher>& other)
{
  wdMath::Swap(this->m_pEntries, other.m_pEntries);
  wdMath::Swap(this->m_pEntryFlags, other.m_pEntryFlags);
  wdMath::Swap(this->m_uiCount, other.m_uiCount);
  wdMath::Swap(this->m_uiCapacity, other.m_uiCapacity);
  wdMath::Swap(this->m_pAllocator, other.m_pAllocator);
}
