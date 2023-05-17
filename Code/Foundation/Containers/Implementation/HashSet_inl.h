
/// \brief Value used by containers for indices to indicate an invalid index.
#ifndef wdInvalidIndex
#  define wdInvalidIndex 0xFFFFFFFF
#endif

// ***** Const Iterator *****

template <typename K, typename H>
wdHashSetBase<K, H>::ConstIterator::ConstIterator(const wdHashSetBase<K, H>& hashSet)
  : m_pHashSet(&hashSet)
{
}

template <typename K, typename H>
void wdHashSetBase<K, H>::ConstIterator::SetToBegin()
{
  if (m_pHashSet->IsEmpty())
  {
    m_uiCurrentIndex = m_pHashSet->m_uiCapacity;
    return;
  }
  while (!m_pHashSet->IsValidEntry(m_uiCurrentIndex))
  {
    ++m_uiCurrentIndex;
  }
}

template <typename K, typename H>
inline void wdHashSetBase<K, H>::ConstIterator::SetToEnd()
{
  m_uiCurrentCount = m_pHashSet->m_uiCount;
  m_uiCurrentIndex = m_pHashSet->m_uiCapacity;
}

template <typename K, typename H>
WD_ALWAYS_INLINE bool wdHashSetBase<K, H>::ConstIterator::IsValid() const
{
  return m_uiCurrentCount < m_pHashSet->m_uiCount;
}

template <typename K, typename H>
WD_ALWAYS_INLINE bool wdHashSetBase<K, H>::ConstIterator::operator==(const typename wdHashSetBase<K, H>::ConstIterator& rhs) const
{
  return m_uiCurrentIndex == rhs.m_uiCurrentIndex && m_pHashSet->m_pEntries == rhs.m_pHashSet->m_pEntries;
}

template <typename K, typename H>
WD_ALWAYS_INLINE bool wdHashSetBase<K, H>::ConstIterator::operator!=(const typename wdHashSetBase<K, H>::ConstIterator& rhs) const
{
  return !(*this == rhs);
}

template <typename K, typename H>
WD_FORCE_INLINE const K& wdHashSetBase<K, H>::ConstIterator::Key() const
{
  return m_pHashSet->m_pEntries[m_uiCurrentIndex];
}

template <typename K, typename H>
void wdHashSetBase<K, H>::ConstIterator::Next()
{
  ++m_uiCurrentCount;
  if (m_uiCurrentCount == m_pHashSet->m_uiCount)
  {
    m_uiCurrentIndex = m_pHashSet->m_uiCapacity;
    return;
  }

  do
  {
    ++m_uiCurrentIndex;
  } while (!m_pHashSet->IsValidEntry(m_uiCurrentIndex));
}

template <typename K, typename H>
WD_ALWAYS_INLINE void wdHashSetBase<K, H>::ConstIterator::operator++()
{
  Next();
}


// ***** wdHashSetBase *****

template <typename K, typename H>
wdHashSetBase<K, H>::wdHashSetBase(wdAllocatorBase* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;
}

template <typename K, typename H>
wdHashSetBase<K, H>::wdHashSetBase(const wdHashSetBase<K, H>& other, wdAllocatorBase* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;

  *this = other;
}

template <typename K, typename H>
wdHashSetBase<K, H>::wdHashSetBase(wdHashSetBase<K, H>&& other, wdAllocatorBase* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;

  *this = std::move(other);
}

template <typename K, typename H>
wdHashSetBase<K, H>::~wdHashSetBase()
{
  Clear();
  WD_DELETE_RAW_BUFFER(m_pAllocator, m_pEntries);
  WD_DELETE_RAW_BUFFER(m_pAllocator, m_pEntryFlags);
  m_uiCapacity = 0;
}

template <typename K, typename H>
void wdHashSetBase<K, H>::operator=(const wdHashSetBase<K, H>& rhs)
{
  Clear();
  Reserve(rhs.GetCount());

  wdUInt32 uiCopied = 0;
  for (wdUInt32 i = 0; uiCopied < rhs.GetCount(); ++i)
  {
    if (rhs.IsValidEntry(i))
    {
      Insert(rhs.m_pEntries[i]);
      ++uiCopied;
    }
  }
}

template <typename K, typename H>
void wdHashSetBase<K, H>::operator=(wdHashSetBase<K, H>&& rhs)
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
        Insert(std::move(rhs.m_pEntries[i]));
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

template <typename K, typename H>
bool wdHashSetBase<K, H>::operator==(const wdHashSetBase<K, H>& rhs) const
{
  if (m_uiCount != rhs.m_uiCount)
    return false;

  wdUInt32 uiCompared = 0;
  for (wdUInt32 i = 0; uiCompared < m_uiCount; ++i)
  {
    if (IsValidEntry(i))
    {
      if (!rhs.Contains(m_pEntries[i]))
        return false;

      ++uiCompared;
    }
  }

  return true;
}

template <typename K, typename H>
WD_ALWAYS_INLINE bool wdHashSetBase<K, H>::operator!=(const wdHashSetBase<K, H>& rhs) const
{
  return !(*this == rhs);
}

template <typename K, typename H>
void wdHashSetBase<K, H>::Reserve(wdUInt32 uiCapacity)
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

template <typename K, typename H>
void wdHashSetBase<K, H>::Compact()
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
    const wdUInt32 uiNewCapacity = (m_uiCount + (CAPACITY_ALIGNMENT - 1)) & ~(CAPACITY_ALIGNMENT - 1);
    if (m_uiCapacity != uiNewCapacity)
      SetCapacity(uiNewCapacity);
  }
}

template <typename K, typename H>
WD_ALWAYS_INLINE wdUInt32 wdHashSetBase<K, H>::GetCount() const
{
  return m_uiCount;
}

template <typename K, typename H>
WD_ALWAYS_INLINE bool wdHashSetBase<K, H>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename K, typename H>
void wdHashSetBase<K, H>::Clear()
{
  for (wdUInt32 i = 0; i < m_uiCapacity; ++i)
  {
    if (IsValidEntry(i))
    {
      wdMemoryUtils::Destruct(&m_pEntries[i], 1);
    }
  }

  wdMemoryUtils::ZeroFill(m_pEntryFlags, GetFlagsCapacity());
  m_uiCount = 0;
}

template <typename K, typename H>
template <typename CompatibleKeyType>
bool wdHashSetBase<K, H>::Insert(CompatibleKeyType&& key)
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
    else if (H::Equal(m_pEntries[uiIndex], key))
    {
      return true;
    }
    ++uiIndex;
    if (uiIndex == m_uiCapacity)
      uiIndex = 0;

    ++uiCounter;
  }

  // new entry
  uiIndex = uiDeletedIndex != wdInvalidIndex ? uiDeletedIndex : uiIndex;

  // Constructions might either be a move or a copy.
  wdMemoryUtils::CopyOrMoveConstruct(&m_pEntries[uiIndex], std::forward<CompatibleKeyType>(key));

  MarkEntryAsValid(uiIndex);
  ++m_uiCount;

  return false;
}

template <typename K, typename H>
template <typename CompatibleKeyType>
bool wdHashSetBase<K, H>::Remove(const CompatibleKeyType& key)
{
  wdUInt32 uiIndex = FindEntry(key);
  if (uiIndex != wdInvalidIndex)
  {
    RemoveInternal(uiIndex);
    return true;
  }

  return false;
}

template <typename K, typename H>
typename wdHashSetBase<K, H>::ConstIterator wdHashSetBase<K, H>::Remove(const typename wdHashSetBase<K, H>::ConstIterator& pos)
{
  ConstIterator it = pos;
  wdUInt32 uiIndex = pos.m_uiCurrentIndex;
  ++it;
  --it.m_uiCurrentCount;
  RemoveInternal(uiIndex);
  return it;
}

template <typename K, typename H>
void wdHashSetBase<K, H>::RemoveInternal(wdUInt32 uiIndex)
{
  wdMemoryUtils::Destruct(&m_pEntries[uiIndex], 1);

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

template <typename K, typename H>
template <typename CompatibleKeyType>
WD_FORCE_INLINE bool wdHashSetBase<K, H>::Contains(const CompatibleKeyType& key) const
{
  return FindEntry(key) != wdInvalidIndex;
}

template <typename K, typename H>
bool wdHashSetBase<K, H>::ContainsSet(const wdHashSetBase<K, H>& operand) const
{
  for (const K& key : operand)
  {
    if (!Contains(key))
      return false;
  }

  return true;
}

template <typename K, typename H>
void wdHashSetBase<K, H>::Union(const wdHashSetBase<K, H>& operand)
{
  Reserve(GetCount() + operand.GetCount());
  for (const auto& key : operand)
  {
    Insert(key);
  }
}

template <typename K, typename H>
void wdHashSetBase<K, H>::Difference(const wdHashSetBase<K, H>& operand)
{
  for (const auto& key : operand)
  {
    Remove(key);
  }
}

template <typename K, typename H>
void wdHashSetBase<K, H>::Intersection(const wdHashSetBase<K, H>& operand)
{
  for (auto it = GetIterator(); it.IsValid();)
  {
    if (!operand.Contains(it.Key()))
      it = Remove(it);
    else
      ++it;
  }
}

template <typename K, typename H>
WD_FORCE_INLINE typename wdHashSetBase<K, H>::ConstIterator wdHashSetBase<K, H>::GetIterator() const
{
  ConstIterator iterator(*this);
  iterator.SetToBegin();
  return iterator;
}

template <typename K, typename H>
WD_FORCE_INLINE typename wdHashSetBase<K, H>::ConstIterator wdHashSetBase<K, H>::GetEndIterator() const
{
  ConstIterator iterator(*this);
  iterator.SetToEnd();
  return iterator;
}

template <typename K, typename H>
WD_ALWAYS_INLINE wdAllocatorBase* wdHashSetBase<K, H>::GetAllocator() const
{
  return m_pAllocator;
}

template <typename K, typename H>
wdUInt64 wdHashSetBase<K, H>::GetHeapMemoryUsage() const
{
  return ((wdUInt64)m_uiCapacity * sizeof(K)) + (sizeof(wdUInt32) * (wdUInt64)GetFlagsCapacity());
}

// private methods
template <typename K, typename H>
void wdHashSetBase<K, H>::SetCapacity(wdUInt32 uiCapacity)
{
  WD_ASSERT_DEV(wdMath::IsPowerOf2(uiCapacity), "uiCapacity must be a power of two to avoid modulo during lookup.");
  const wdUInt32 uiOldCapacity = m_uiCapacity;
  m_uiCapacity = uiCapacity;

  K* pOldEntries = m_pEntries;
  wdUInt32* pOldEntryFlags = m_pEntryFlags;

  m_pEntries = WD_NEW_RAW_BUFFER(m_pAllocator, K, m_uiCapacity);
  m_pEntryFlags = WD_NEW_RAW_BUFFER(m_pAllocator, wdUInt32, GetFlagsCapacity());
  wdMemoryUtils::ZeroFill(m_pEntryFlags, GetFlagsCapacity());

  m_uiCount = 0;
  for (wdUInt32 i = 0; i < uiOldCapacity; ++i)
  {
    if (GetFlags(pOldEntryFlags, i) == VALID_ENTRY)
    {
      WD_VERIFY(!Insert(std::move(pOldEntries[i])), "Implementation error");

      wdMemoryUtils::Destruct(&pOldEntries[i], 1);
    }
  }

  WD_DELETE_RAW_BUFFER(m_pAllocator, pOldEntries);
  WD_DELETE_RAW_BUFFER(m_pAllocator, pOldEntryFlags);
}

template <typename K, typename H>
template <typename CompatibleKeyType>
WD_FORCE_INLINE wdUInt32 wdHashSetBase<K, H>::FindEntry(const CompatibleKeyType& key) const
{
  return FindEntry(H::Hash(key), key);
}

template <typename K, typename H>
template <typename CompatibleKeyType>
inline wdUInt32 wdHashSetBase<K, H>::FindEntry(wdUInt32 uiHash, const CompatibleKeyType& key) const
{
  if (m_uiCapacity > 0)
  {
    wdUInt32 uiIndex = uiHash & (m_uiCapacity - 1);
    wdUInt32 uiCounter = 0;
    while (!IsFreeEntry(uiIndex) && uiCounter < m_uiCapacity)
    {
      if (IsValidEntry(uiIndex) && H::Equal(m_pEntries[uiIndex], key))
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

#define WD_HASHSET_USE_BITFLAGS WD_ON

template <typename K, typename H>
WD_FORCE_INLINE wdUInt32 wdHashSetBase<K, H>::GetFlagsCapacity() const
{
#if WD_ENABLED(WD_HASHSET_USE_BITFLAGS)
  return (m_uiCapacity + 15) / 16;
#else
  return m_uiCapacity;
#endif
}

template <typename K, typename H>
wdUInt32 wdHashSetBase<K, H>::GetFlags(wdUInt32* pFlags, wdUInt32 uiEntryIndex) const
{
#if WD_ENABLED(WD_HASHSET_USE_BITFLAGS)
  const wdUInt32 uiIndex = uiEntryIndex / 16;
  const wdUInt32 uiSubIndex = (uiEntryIndex & 15) * 2;
  return (pFlags[uiIndex] >> uiSubIndex) & FLAGS_MASK;
#else
  return pFlags[uiEntryIndex] & FLAGS_MASK;
#endif
}

template <typename K, typename H>
void wdHashSetBase<K, H>::SetFlags(wdUInt32 uiEntryIndex, wdUInt32 uiFlags)
{
#if WD_ENABLED(WD_HASHSET_USE_BITFLAGS)
  const wdUInt32 uiIndex = uiEntryIndex / 16;
  const wdUInt32 uiSubIndex = (uiEntryIndex & 15) * 2;
  WD_ASSERT_DEV(uiIndex < GetFlagsCapacity(), "Out of bounds access");
  m_pEntryFlags[uiIndex] &= ~(FLAGS_MASK << uiSubIndex);
  m_pEntryFlags[uiIndex] |= (uiFlags << uiSubIndex);
#else
  WD_ASSERT_DEV(uiEntryIndex < GetFlagsCapacity(), "Out of bounds access");
  m_pEntryFlags[uiEntryIndex] = uiFlags;
#endif
}

template <typename K, typename H>
WD_FORCE_INLINE bool wdHashSetBase<K, H>::IsFreeEntry(wdUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == FREE_ENTRY;
}

template <typename K, typename H>
WD_FORCE_INLINE bool wdHashSetBase<K, H>::IsValidEntry(wdUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == VALID_ENTRY;
}

template <typename K, typename H>
WD_FORCE_INLINE bool wdHashSetBase<K, H>::IsDeletedEntry(wdUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == DELETED_ENTRY;
}

template <typename K, typename H>
WD_FORCE_INLINE void wdHashSetBase<K, H>::MarkEntryAsFree(wdUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, FREE_ENTRY);
}

template <typename K, typename H>
WD_FORCE_INLINE void wdHashSetBase<K, H>::MarkEntryAsValid(wdUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, VALID_ENTRY);
}

template <typename K, typename H>
WD_FORCE_INLINE void wdHashSetBase<K, H>::MarkEntryAsDeleted(wdUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, DELETED_ENTRY);
}


template <typename K, typename H, typename A>
wdHashSet<K, H, A>::wdHashSet()
  : wdHashSetBase<K, H>(A::GetAllocator())
{
}

template <typename K, typename H, typename A>
wdHashSet<K, H, A>::wdHashSet(wdAllocatorBase* pAllocator)
  : wdHashSetBase<K, H>(pAllocator)
{
}

template <typename K, typename H, typename A>
wdHashSet<K, H, A>::wdHashSet(const wdHashSet<K, H, A>& other)
  : wdHashSetBase<K, H>(other, A::GetAllocator())
{
}

template <typename K, typename H, typename A>
wdHashSet<K, H, A>::wdHashSet(const wdHashSetBase<K, H>& other)
  : wdHashSetBase<K, H>(other, A::GetAllocator())
{
}

template <typename K, typename H, typename A>
wdHashSet<K, H, A>::wdHashSet(wdHashSet<K, H, A>&& other)
  : wdHashSetBase<K, H>(std::move(other), other.GetAllocator())
{
}

template <typename K, typename H, typename A>
wdHashSet<K, H, A>::wdHashSet(wdHashSetBase<K, H>&& other)
  : wdHashSetBase<K, H>(std::move(other), other.GetAllocator())
{
}

template <typename K, typename H, typename A>
void wdHashSet<K, H, A>::operator=(const wdHashSet<K, H, A>& rhs)
{
  wdHashSetBase<K, H>::operator=(rhs);
}

template <typename K, typename H, typename A>
void wdHashSet<K, H, A>::operator=(const wdHashSetBase<K, H>& rhs)
{
  wdHashSetBase<K, H>::operator=(rhs);
}

template <typename K, typename H, typename A>
void wdHashSet<K, H, A>::operator=(wdHashSet<K, H, A>&& rhs)
{
  wdHashSetBase<K, H>::operator=(std::move(rhs));
}

template <typename K, typename H, typename A>
void wdHashSet<K, H, A>::operator=(wdHashSetBase<K, H>&& rhs)
{
  wdHashSetBase<K, H>::operator=(std::move(rhs));
}

template <typename KeyType, typename Hasher>
void wdHashSetBase<KeyType, Hasher>::Swap(wdHashSetBase<KeyType, Hasher>& other)
{
  wdMath::Swap(this->m_pEntries, other.m_pEntries);
  wdMath::Swap(this->m_pEntryFlags, other.m_pEntryFlags);
  wdMath::Swap(this->m_uiCount, other.m_uiCount);
  wdMath::Swap(this->m_uiCapacity, other.m_uiCapacity);
  wdMath::Swap(this->m_pAllocator, other.m_pAllocator);
}
