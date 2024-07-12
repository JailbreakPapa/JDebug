
/// \brief Value used by containers for indices to indicate an invalid index.
#ifndef nsInvalidIndex
#  define nsInvalidIndex 0xFFFFFFFF
#endif

// ***** Const Iterator *****

template <typename K, typename H>
nsHashSetBase<K, H>::ConstIterator::ConstIterator(const nsHashSetBase<K, H>& hashSet)
  : m_pHashSet(&hashSet)
{
}

template <typename K, typename H>
void nsHashSetBase<K, H>::ConstIterator::SetToBegin()
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
inline void nsHashSetBase<K, H>::ConstIterator::SetToEnd()
{
  m_uiCurrentCount = m_pHashSet->m_uiCount;
  m_uiCurrentIndex = m_pHashSet->m_uiCapacity;
}

template <typename K, typename H>
NS_ALWAYS_INLINE bool nsHashSetBase<K, H>::ConstIterator::IsValid() const
{
  return m_uiCurrentCount < m_pHashSet->m_uiCount;
}

template <typename K, typename H>
NS_ALWAYS_INLINE bool nsHashSetBase<K, H>::ConstIterator::operator==(const typename nsHashSetBase<K, H>::ConstIterator& rhs) const
{
  return m_uiCurrentIndex == rhs.m_uiCurrentIndex && m_pHashSet->m_pEntries == rhs.m_pHashSet->m_pEntries;
}

template <typename K, typename H>
NS_FORCE_INLINE const K& nsHashSetBase<K, H>::ConstIterator::Key() const
{
  return m_pHashSet->m_pEntries[m_uiCurrentIndex];
}

template <typename K, typename H>
void nsHashSetBase<K, H>::ConstIterator::Next()
{
  ++m_uiCurrentCount;
  if (m_uiCurrentCount == m_pHashSet->m_uiCount)
  {
    m_uiCurrentIndex = m_pHashSet->m_uiCapacity;
    return;
  }

  for (++m_uiCurrentIndex; m_uiCurrentIndex < m_pHashSet->m_uiCapacity; ++m_uiCurrentIndex)
  {
    if (m_pHashSet->IsValidEntry(m_uiCurrentIndex))
    {
      return;
    }
  }
  SetToEnd();
}

template <typename K, typename H>
NS_ALWAYS_INLINE void nsHashSetBase<K, H>::ConstIterator::operator++()
{
  Next();
}


// ***** nsHashSetBase *****

template <typename K, typename H>
nsHashSetBase<K, H>::nsHashSetBase(nsAllocator* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;
}

template <typename K, typename H>
nsHashSetBase<K, H>::nsHashSetBase(const nsHashSetBase<K, H>& other, nsAllocator* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;

  *this = other;
}

template <typename K, typename H>
nsHashSetBase<K, H>::nsHashSetBase(nsHashSetBase<K, H>&& other, nsAllocator* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;

  *this = std::move(other);
}

template <typename K, typename H>
nsHashSetBase<K, H>::~nsHashSetBase()
{
  Clear();
  NS_DELETE_RAW_BUFFER(m_pAllocator, m_pEntries);
  NS_DELETE_RAW_BUFFER(m_pAllocator, m_pEntryFlags);
  m_uiCapacity = 0;
}

template <typename K, typename H>
void nsHashSetBase<K, H>::operator=(const nsHashSetBase<K, H>& rhs)
{
  Clear();
  Reserve(rhs.GetCount());

  nsUInt32 uiCopied = 0;
  for (nsUInt32 i = 0; uiCopied < rhs.GetCount(); ++i)
  {
    if (rhs.IsValidEntry(i))
    {
      Insert(rhs.m_pEntries[i]);
      ++uiCopied;
    }
  }
}

template <typename K, typename H>
void nsHashSetBase<K, H>::operator=(nsHashSetBase<K, H>&& rhs)
{
  // Clear any existing data (calls destructors if necessary)
  Clear();

  if (m_pAllocator != rhs.m_pAllocator)
  {
    Reserve(rhs.m_uiCapacity);

    nsUInt32 uiCopied = 0;
    for (nsUInt32 i = 0; uiCopied < rhs.GetCount(); ++i)
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
    NS_DELETE_RAW_BUFFER(m_pAllocator, m_pEntries);
    NS_DELETE_RAW_BUFFER(m_pAllocator, m_pEntryFlags);

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
bool nsHashSetBase<K, H>::operator==(const nsHashSetBase<K, H>& rhs) const
{
  if (m_uiCount != rhs.m_uiCount)
    return false;

  nsUInt32 uiCompared = 0;
  for (nsUInt32 i = 0; uiCompared < m_uiCount; ++i)
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
void nsHashSetBase<K, H>::Reserve(nsUInt32 uiCapacity)
{
  const nsUInt64 uiCap64 = static_cast<nsUInt64>(uiCapacity);
  nsUInt64 uiNewCapacity64 = uiCap64 + (uiCap64 * 2 / 3);                  // ensure a maximum load of 60%

  uiNewCapacity64 = nsMath::Min<nsUInt64>(uiNewCapacity64, 0x80000000llu); // the largest power-of-two in 32 bit

  nsUInt32 uiNewCapacity32 = static_cast<nsUInt32>(uiNewCapacity64 & 0xFFFFFFFF);
  NS_ASSERT_DEBUG(uiCapacity <= uiNewCapacity32, "nsHashSet/Map do not support more than 2 billion entries.");

  if (m_uiCapacity >= uiNewCapacity32)
    return;

  uiNewCapacity32 = nsMath::Max<nsUInt32>(nsMath::PowerOfTwo_Ceil(uiNewCapacity32), CAPACITY_ALIGNMENT);
  SetCapacity(uiNewCapacity32);
}

template <typename K, typename H>
void nsHashSetBase<K, H>::Compact()
{
  if (IsEmpty())
  {
    // completely deallocate all data, if the table is empty.
    NS_DELETE_RAW_BUFFER(m_pAllocator, m_pEntries);
    NS_DELETE_RAW_BUFFER(m_pAllocator, m_pEntryFlags);
    m_uiCapacity = 0;
  }
  else
  {
    const nsUInt32 uiNewCapacity = (m_uiCount + (CAPACITY_ALIGNMENT - 1)) & ~(CAPACITY_ALIGNMENT - 1);
    if (m_uiCapacity != uiNewCapacity)
      SetCapacity(uiNewCapacity);
  }
}

template <typename K, typename H>
NS_ALWAYS_INLINE nsUInt32 nsHashSetBase<K, H>::GetCount() const
{
  return m_uiCount;
}

template <typename K, typename H>
NS_ALWAYS_INLINE bool nsHashSetBase<K, H>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename K, typename H>
void nsHashSetBase<K, H>::Clear()
{
  for (nsUInt32 i = 0; i < m_uiCapacity; ++i)
  {
    if (IsValidEntry(i))
    {
      nsMemoryUtils::Destruct(&m_pEntries[i], 1);
    }
  }

  nsMemoryUtils::ZeroFill(m_pEntryFlags, GetFlagsCapacity());
  m_uiCount = 0;
}

template <typename K, typename H>
template <typename CompatibleKeyType>
bool nsHashSetBase<K, H>::Insert(CompatibleKeyType&& key)
{
  Reserve(m_uiCount + 1);

  nsUInt32 uiIndex = H::Hash(key) & (m_uiCapacity - 1);
  nsUInt32 uiDeletedIndex = nsInvalidIndex;

  nsUInt32 uiCounter = 0;
  while (!IsFreeEntry(uiIndex) && uiCounter < m_uiCapacity)
  {
    if (IsDeletedEntry(uiIndex))
    {
      if (uiDeletedIndex == nsInvalidIndex)
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
  uiIndex = uiDeletedIndex != nsInvalidIndex ? uiDeletedIndex : uiIndex;

  // Constructions might either be a move or a copy.
  nsMemoryUtils::CopyOrMoveConstruct(&m_pEntries[uiIndex], std::forward<CompatibleKeyType>(key));

  MarkEntryAsValid(uiIndex);
  ++m_uiCount;

  return false;
}

template <typename K, typename H>
template <typename CompatibleKeyType>
bool nsHashSetBase<K, H>::Remove(const CompatibleKeyType& key)
{
  nsUInt32 uiIndex = FindEntry(key);
  if (uiIndex != nsInvalidIndex)
  {
    RemoveInternal(uiIndex);
    return true;
  }

  return false;
}

template <typename K, typename H>
typename nsHashSetBase<K, H>::ConstIterator nsHashSetBase<K, H>::Remove(const typename nsHashSetBase<K, H>::ConstIterator& pos)
{
  ConstIterator it = pos;
  nsUInt32 uiIndex = pos.m_uiCurrentIndex;
  ++it;
  --it.m_uiCurrentCount;
  RemoveInternal(uiIndex);
  return it;
}

template <typename K, typename H>
void nsHashSetBase<K, H>::RemoveInternal(nsUInt32 uiIndex)
{
  nsMemoryUtils::Destruct(&m_pEntries[uiIndex], 1);

  nsUInt32 uiNextIndex = uiIndex + 1;
  if (uiNextIndex == m_uiCapacity)
    uiNextIndex = 0;

  // if the next entry is free we are at the end of a chain and
  // can immediately mark this entry as free as well
  if (IsFreeEntry(uiNextIndex))
  {
    MarkEntryAsFree(uiIndex);

    // run backwards and free all deleted entries in this chain
    nsUInt32 uiPrevIndex = (uiIndex != 0) ? uiIndex : m_uiCapacity;
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
NS_FORCE_INLINE bool nsHashSetBase<K, H>::Contains(const CompatibleKeyType& key) const
{
  return FindEntry(key) != nsInvalidIndex;
}

template <typename K, typename H>
template <typename CompatibleKeyType>
NS_FORCE_INLINE typename nsHashSetBase<K, H>::ConstIterator nsHashSetBase<K, H>::Find(const CompatibleKeyType& key) const
{
  nsUInt32 uiIndex = FindEntry(key);
  if (uiIndex == nsInvalidIndex)
  {
    return GetEndIterator();
  }

  ConstIterator it(*this);
  it.m_uiCurrentIndex = uiIndex;
  it.m_uiCurrentCount = 0; // we do not know the 'count' (which is used as an optimization), so we just use 0

  return it;
}

template <typename K, typename H>
bool nsHashSetBase<K, H>::ContainsSet(const nsHashSetBase<K, H>& operand) const
{
  for (const K& key : operand)
  {
    if (!Contains(key))
      return false;
  }

  return true;
}

template <typename K, typename H>
void nsHashSetBase<K, H>::Union(const nsHashSetBase<K, H>& operand)
{
  Reserve(GetCount() + operand.GetCount());
  for (const auto& key : operand)
  {
    Insert(key);
  }
}

template <typename K, typename H>
void nsHashSetBase<K, H>::Difference(const nsHashSetBase<K, H>& operand)
{
  for (const auto& key : operand)
  {
    Remove(key);
  }
}

template <typename K, typename H>
void nsHashSetBase<K, H>::Intersection(const nsHashSetBase<K, H>& operand)
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
NS_FORCE_INLINE typename nsHashSetBase<K, H>::ConstIterator nsHashSetBase<K, H>::GetIterator() const
{
  ConstIterator iterator(*this);
  iterator.SetToBegin();
  return iterator;
}

template <typename K, typename H>
NS_FORCE_INLINE typename nsHashSetBase<K, H>::ConstIterator nsHashSetBase<K, H>::GetEndIterator() const
{
  ConstIterator iterator(*this);
  iterator.SetToEnd();
  return iterator;
}

template <typename K, typename H>
NS_ALWAYS_INLINE nsAllocator* nsHashSetBase<K, H>::GetAllocator() const
{
  return m_pAllocator;
}

template <typename K, typename H>
nsUInt64 nsHashSetBase<K, H>::GetHeapMemoryUsage() const
{
  return ((nsUInt64)m_uiCapacity * sizeof(K)) + (sizeof(nsUInt32) * (nsUInt64)GetFlagsCapacity());
}

// private methods
template <typename K, typename H>
void nsHashSetBase<K, H>::SetCapacity(nsUInt32 uiCapacity)
{
  NS_ASSERT_DEBUG(nsMath::IsPowerOf2(uiCapacity), "uiCapacity must be a power of two to avoid modulo during lookup.");
  const nsUInt32 uiOldCapacity = m_uiCapacity;
  m_uiCapacity = uiCapacity;

  K* pOldEntries = m_pEntries;
  nsUInt32* pOldEntryFlags = m_pEntryFlags;

  m_pEntries = NS_NEW_RAW_BUFFER(m_pAllocator, K, m_uiCapacity);
  m_pEntryFlags = NS_NEW_RAW_BUFFER(m_pAllocator, nsUInt32, GetFlagsCapacity());
  nsMemoryUtils::ZeroFill(m_pEntryFlags, GetFlagsCapacity());

  m_uiCount = 0;
  for (nsUInt32 i = 0; i < uiOldCapacity; ++i)
  {
    if (GetFlags(pOldEntryFlags, i) == VALID_ENTRY)
    {
      NS_VERIFY(!Insert(std::move(pOldEntries[i])), "Implementation error");

      nsMemoryUtils::Destruct(&pOldEntries[i], 1);
    }
  }

  NS_DELETE_RAW_BUFFER(m_pAllocator, pOldEntries);
  NS_DELETE_RAW_BUFFER(m_pAllocator, pOldEntryFlags);
}

template <typename K, typename H>
template <typename CompatibleKeyType>
NS_FORCE_INLINE nsUInt32 nsHashSetBase<K, H>::FindEntry(const CompatibleKeyType& key) const
{
  return FindEntry(H::Hash(key), key);
}

template <typename K, typename H>
template <typename CompatibleKeyType>
inline nsUInt32 nsHashSetBase<K, H>::FindEntry(nsUInt32 uiHash, const CompatibleKeyType& key) const
{
  if (m_uiCapacity > 0)
  {
    nsUInt32 uiIndex = uiHash & (m_uiCapacity - 1);
    nsUInt32 uiCounter = 0;
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
  return nsInvalidIndex;
}

#define NS_HASHSET_USE_BITFLAGS NS_ON

template <typename K, typename H>
NS_FORCE_INLINE nsUInt32 nsHashSetBase<K, H>::GetFlagsCapacity() const
{
#if NS_ENABLED(NS_HASHSET_USE_BITFLAGS)
  return (m_uiCapacity + 15) / 16;
#else
  return m_uiCapacity;
#endif
}

template <typename K, typename H>
nsUInt32 nsHashSetBase<K, H>::GetFlags(nsUInt32* pFlags, nsUInt32 uiEntryIndex) const
{
#if NS_ENABLED(NS_HASHSET_USE_BITFLAGS)
  const nsUInt32 uiIndex = uiEntryIndex / 16;
  const nsUInt32 uiSubIndex = (uiEntryIndex & 15) * 2;
  return (pFlags[uiIndex] >> uiSubIndex) & FLAGS_MASK;
#else
  return pFlags[uiEntryIndex] & FLAGS_MASK;
#endif
}

template <typename K, typename H>
void nsHashSetBase<K, H>::SetFlags(nsUInt32 uiEntryIndex, nsUInt32 uiFlags)
{
#if NS_ENABLED(NS_HASHSET_USE_BITFLAGS)
  const nsUInt32 uiIndex = uiEntryIndex / 16;
  const nsUInt32 uiSubIndex = (uiEntryIndex & 15) * 2;
  NS_ASSERT_DEBUG(uiIndex < GetFlagsCapacity(), "Out of bounds access");
  m_pEntryFlags[uiIndex] &= ~(FLAGS_MASK << uiSubIndex);
  m_pEntryFlags[uiIndex] |= (uiFlags << uiSubIndex);
#else
  NS_ASSERT_DEBUG(uiEntryIndex < GetFlagsCapacity(), "Out of bounds access");
  m_pEntryFlags[uiEntryIndex] = uiFlags;
#endif
}

template <typename K, typename H>
NS_FORCE_INLINE bool nsHashSetBase<K, H>::IsFreeEntry(nsUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == FREE_ENTRY;
}

template <typename K, typename H>
NS_FORCE_INLINE bool nsHashSetBase<K, H>::IsValidEntry(nsUInt32 uiEntryIndex) const
{
  NS_ASSERT_DEBUG(uiEntryIndex < m_uiCapacity, "Out of bounds access");
  return GetFlags(m_pEntryFlags, uiEntryIndex) == VALID_ENTRY;
}

template <typename K, typename H>
NS_FORCE_INLINE bool nsHashSetBase<K, H>::IsDeletedEntry(nsUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == DELETED_ENTRY;
}

template <typename K, typename H>
NS_FORCE_INLINE void nsHashSetBase<K, H>::MarkEntryAsFree(nsUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, FREE_ENTRY);
}

template <typename K, typename H>
NS_FORCE_INLINE void nsHashSetBase<K, H>::MarkEntryAsValid(nsUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, VALID_ENTRY);
}

template <typename K, typename H>
NS_FORCE_INLINE void nsHashSetBase<K, H>::MarkEntryAsDeleted(nsUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, DELETED_ENTRY);
}


template <typename K, typename H, typename A>
nsHashSet<K, H, A>::nsHashSet()
  : nsHashSetBase<K, H>(A::GetAllocator())
{
}

template <typename K, typename H, typename A>
nsHashSet<K, H, A>::nsHashSet(nsAllocator* pAllocator)
  : nsHashSetBase<K, H>(pAllocator)
{
}

template <typename K, typename H, typename A>
nsHashSet<K, H, A>::nsHashSet(const nsHashSet<K, H, A>& other)
  : nsHashSetBase<K, H>(other, A::GetAllocator())
{
}

template <typename K, typename H, typename A>
nsHashSet<K, H, A>::nsHashSet(const nsHashSetBase<K, H>& other)
  : nsHashSetBase<K, H>(other, A::GetAllocator())
{
}

template <typename K, typename H, typename A>
nsHashSet<K, H, A>::nsHashSet(nsHashSet<K, H, A>&& other)
  : nsHashSetBase<K, H>(std::move(other), other.GetAllocator())
{
}

template <typename K, typename H, typename A>
nsHashSet<K, H, A>::nsHashSet(nsHashSetBase<K, H>&& other)
  : nsHashSetBase<K, H>(std::move(other), other.GetAllocator())
{
}

template <typename K, typename H, typename A>
void nsHashSet<K, H, A>::operator=(const nsHashSet<K, H, A>& rhs)
{
  nsHashSetBase<K, H>::operator=(rhs);
}

template <typename K, typename H, typename A>
void nsHashSet<K, H, A>::operator=(const nsHashSetBase<K, H>& rhs)
{
  nsHashSetBase<K, H>::operator=(rhs);
}

template <typename K, typename H, typename A>
void nsHashSet<K, H, A>::operator=(nsHashSet<K, H, A>&& rhs)
{
  nsHashSetBase<K, H>::operator=(std::move(rhs));
}

template <typename K, typename H, typename A>
void nsHashSet<K, H, A>::operator=(nsHashSetBase<K, H>&& rhs)
{
  nsHashSetBase<K, H>::operator=(std::move(rhs));
}

template <typename KeyType, typename Hasher>
void nsHashSetBase<KeyType, Hasher>::Swap(nsHashSetBase<KeyType, Hasher>& other)
{
  nsMath::Swap(this->m_pEntries, other.m_pEntries);
  nsMath::Swap(this->m_pEntryFlags, other.m_pEntryFlags);
  nsMath::Swap(this->m_uiCount, other.m_uiCount);
  nsMath::Swap(this->m_uiCapacity, other.m_uiCapacity);
  nsMath::Swap(this->m_pAllocator, other.m_pAllocator);
}
