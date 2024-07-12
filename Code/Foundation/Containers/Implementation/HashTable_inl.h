
/// \brief Value used by containers for indices to indicate an invalid index.
#ifndef nsInvalidIndex
#  define nsInvalidIndex 0xFFFFFFFF
#endif

// ***** Const Iterator *****

template <typename K, typename V, typename H>
nsHashTableBaseConstIterator<K, V, H>::nsHashTableBaseConstIterator(const nsHashTableBase<K, V, H>& hashTable)
  : m_pHashTable(&hashTable)
{
}

template <typename K, typename V, typename H>
void nsHashTableBaseConstIterator<K, V, H>::SetToBegin()
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
inline void nsHashTableBaseConstIterator<K, V, H>::SetToEnd()
{
  m_uiCurrentCount = m_pHashTable->m_uiCount;
  m_uiCurrentIndex = m_pHashTable->m_uiCapacity;
}


template <typename K, typename V, typename H>
NS_FORCE_INLINE bool nsHashTableBaseConstIterator<K, V, H>::IsValid() const
{
  return m_uiCurrentCount < m_pHashTable->m_uiCount;
}

template <typename K, typename V, typename H>
NS_FORCE_INLINE bool nsHashTableBaseConstIterator<K, V, H>::operator==(const nsHashTableBaseConstIterator<K, V, H>& rhs) const
{
  return m_uiCurrentIndex == rhs.m_uiCurrentIndex && m_pHashTable->m_pEntries == rhs.m_pHashTable->m_pEntries;
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE const K& nsHashTableBaseConstIterator<K, V, H>::Key() const
{
  return m_pHashTable->m_pEntries[m_uiCurrentIndex].key;
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE const V& nsHashTableBaseConstIterator<K, V, H>::Value() const
{
  return m_pHashTable->m_pEntries[m_uiCurrentIndex].value;
}

template <typename K, typename V, typename H>
void nsHashTableBaseConstIterator<K, V, H>::Next()
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
NS_ALWAYS_INLINE void nsHashTableBaseConstIterator<K, V, H>::operator++()
{
  Next();
}

#if NS_ENABLED(NS_USE_CPP20_OPERATORS)
// These functions are used for structured bindings.
// They describe how many elements can be accessed in the binding and which type they are.
namespace std
{
  template <typename K, typename V, typename H>
  struct tuple_size<nsHashTableBaseConstIterator<K, V, H>> : integral_constant<size_t, 2>
  {
  };

  template <typename K, typename V, typename H>
  struct tuple_element<0, nsHashTableBaseConstIterator<K, V, H>>
  {
    using type = const K&;
  };

  template <typename K, typename V, typename H>
  struct tuple_element<1, nsHashTableBaseConstIterator<K, V, H>>
  {
    using type = const V&;
  };
} // namespace std
#endif

// ***** Iterator *****

template <typename K, typename V, typename H>
nsHashTableBaseIterator<K, V, H>::nsHashTableBaseIterator(const nsHashTableBase<K, V, H>& hashTable)
  : nsHashTableBaseConstIterator<K, V, H>(hashTable)
{
}

template <typename K, typename V, typename H>
nsHashTableBaseIterator<K, V, H>::nsHashTableBaseIterator(const nsHashTableBaseIterator<K, V, H>& rhs)
  : nsHashTableBaseConstIterator<K, V, H>(*rhs.m_pHashTable)
{
  this->m_uiCurrentIndex = rhs.m_uiCurrentIndex;
  this->m_uiCurrentCount = rhs.m_uiCurrentCount;
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE void nsHashTableBaseIterator<K, V, H>::operator=(const nsHashTableBaseIterator& rhs) // [tested]
{
  this->m_pHashTable = rhs.m_pHashTable;
  this->m_uiCurrentIndex = rhs.m_uiCurrentIndex;
  this->m_uiCurrentCount = rhs.m_uiCurrentCount;
}

template <typename K, typename V, typename H>
NS_FORCE_INLINE V& nsHashTableBaseIterator<K, V, H>::Value()
{
  return this->m_pHashTable->m_pEntries[this->m_uiCurrentIndex].value;
}

template <typename K, typename V, typename H>
NS_FORCE_INLINE V& nsHashTableBaseIterator<K, V, H>::Value() const
{
  return this->m_pHashTable->m_pEntries[this->m_uiCurrentIndex].value;
}


#if NS_ENABLED(NS_USE_CPP20_OPERATORS)
// These functions are used for structured bindings.
// They describe how many elements can be accessed in the binding and which type they are.
namespace std
{
  template <typename K, typename V, typename H>
  struct tuple_size<nsHashTableBaseIterator<K, V, H>> : integral_constant<size_t, 2>
  {
  };

  template <typename K, typename V, typename H>
  struct tuple_element<0, nsHashTableBaseIterator<K, V, H>>
  {
    using type = const K&;
  };

  template <typename K, typename V, typename H>
  struct tuple_element<1, nsHashTableBaseIterator<K, V, H>>
  {
    using type = V&;
  };
} // namespace std
#endif

// ***** nsHashTableBase *****

template <typename K, typename V, typename H>
nsHashTableBase<K, V, H>::nsHashTableBase(nsAllocator* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;
}

template <typename K, typename V, typename H>
nsHashTableBase<K, V, H>::nsHashTableBase(const nsHashTableBase<K, V, H>& other, nsAllocator* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;

  *this = other;
}

template <typename K, typename V, typename H>
nsHashTableBase<K, V, H>::nsHashTableBase(nsHashTableBase<K, V, H>&& other, nsAllocator* pAllocator)
{
  m_pEntries = nullptr;
  m_pEntryFlags = nullptr;
  m_uiCount = 0;
  m_uiCapacity = 0;
  m_pAllocator = pAllocator;

  *this = std::move(other);
}

template <typename K, typename V, typename H>
nsHashTableBase<K, V, H>::~nsHashTableBase()
{
  Clear();
  NS_DELETE_RAW_BUFFER(m_pAllocator, m_pEntries);
  NS_DELETE_RAW_BUFFER(m_pAllocator, m_pEntryFlags);
  m_uiCapacity = 0;
}

template <typename K, typename V, typename H>
void nsHashTableBase<K, V, H>::operator=(const nsHashTableBase<K, V, H>& rhs)
{
  Clear();
  Reserve(rhs.GetCount());

  nsUInt32 uiCopied = 0;
  for (nsUInt32 i = 0; uiCopied < rhs.GetCount(); ++i)
  {
    if (rhs.IsValidEntry(i))
    {
      Insert(rhs.m_pEntries[i].key, rhs.m_pEntries[i].value);
      ++uiCopied;
    }
  }
}

template <typename K, typename V, typename H>
void nsHashTableBase<K, V, H>::operator=(nsHashTableBase<K, V, H>&& rhs)
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
        Insert(std::move(rhs.m_pEntries[i].key), std::move(rhs.m_pEntries[i].value));
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

template <typename K, typename V, typename H>
bool nsHashTableBase<K, V, H>::operator==(const nsHashTableBase<K, V, H>& rhs) const
{
  if (m_uiCount != rhs.m_uiCount)
    return false;

  nsUInt32 uiCompared = 0;
  for (nsUInt32 i = 0; uiCompared < m_uiCount; ++i)
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
void nsHashTableBase<K, V, H>::Reserve(nsUInt32 uiCapacity)
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

template <typename K, typename V, typename H>
void nsHashTableBase<K, V, H>::Compact()
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
    const nsUInt32 uiNewCapacity = nsMath::PowerOfTwo_Ceil(m_uiCount + (CAPACITY_ALIGNMENT - 1)) & ~(CAPACITY_ALIGNMENT - 1);
    if (m_uiCapacity != uiNewCapacity)
      SetCapacity(uiNewCapacity);
  }
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE nsUInt32 nsHashTableBase<K, V, H>::GetCount() const
{
  return m_uiCount;
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE bool nsHashTableBase<K, V, H>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename K, typename V, typename H>
void nsHashTableBase<K, V, H>::Clear()
{
  for (nsUInt32 i = 0; i < m_uiCapacity; ++i)
  {
    if (IsValidEntry(i))
    {
      nsMemoryUtils::Destruct(&m_pEntries[i].key, 1);
      nsMemoryUtils::Destruct(&m_pEntries[i].value, 1);
    }
  }

  nsMemoryUtils::ZeroFill(m_pEntryFlags, GetFlagsCapacity());
  m_uiCount = 0;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType, typename CompatibleValueType>
bool nsHashTableBase<K, V, H>::Insert(CompatibleKeyType&& key, CompatibleValueType&& value, V* out_pOldValue /*= nullptr*/)
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
  uiIndex = uiDeletedIndex != nsInvalidIndex ? uiDeletedIndex : uiIndex;

  // Both constructions might either be a move or a copy.
  nsMemoryUtils::CopyOrMoveConstruct(&m_pEntries[uiIndex].key, std::forward<CompatibleKeyType>(key));
  nsMemoryUtils::CopyOrMoveConstruct(&m_pEntries[uiIndex].value, std::forward<CompatibleValueType>(value));

  MarkEntryAsValid(uiIndex);
  ++m_uiCount;

  return false;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
bool nsHashTableBase<K, V, H>::Remove(const CompatibleKeyType& key, V* out_pOldValue /*= nullptr*/)
{
  nsUInt32 uiIndex = FindEntry(key);
  if (uiIndex != nsInvalidIndex)
  {
    if (out_pOldValue != nullptr)
      *out_pOldValue = std::move(m_pEntries[uiIndex].value);

    RemoveInternal(uiIndex);
    return true;
  }

  return false;
}

template <typename K, typename V, typename H>
typename nsHashTableBase<K, V, H>::Iterator nsHashTableBase<K, V, H>::Remove(const typename nsHashTableBase<K, V, H>::Iterator& pos)
{
  NS_ASSERT_DEBUG(pos.m_pHashTable == this, "Iterator from wrong hashtable");
  Iterator it = pos;
  nsUInt32 uiIndex = pos.m_uiCurrentIndex;
  ++it;
  --it.m_uiCurrentCount;
  RemoveInternal(uiIndex);
  return it;
}

template <typename K, typename V, typename H>
void nsHashTableBase<K, V, H>::RemoveInternal(nsUInt32 uiIndex)
{
  nsMemoryUtils::Destruct(&m_pEntries[uiIndex].key, 1);
  nsMemoryUtils::Destruct(&m_pEntries[uiIndex].value, 1);

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

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline bool nsHashTableBase<K, V, H>::TryGetValue(const CompatibleKeyType& key, V& out_value) const
{
  nsUInt32 uiIndex = FindEntry(key);
  if (uiIndex != nsInvalidIndex)
  {
    NS_ASSERT_DEBUG(m_pEntries != nullptr, "No entries present"); // To fix static analysis
    out_value = m_pEntries[uiIndex].value;
    return true;
  }

  return false;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline bool nsHashTableBase<K, V, H>::TryGetValue(const CompatibleKeyType& key, const V*& out_pValue) const
{
  nsUInt32 uiIndex = FindEntry(key);
  if (uiIndex != nsInvalidIndex)
  {
    out_pValue = &m_pEntries[uiIndex].value;
    NS_ANALYSIS_ASSUME(out_pValue != nullptr);
    return true;
  }

  return false;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline bool nsHashTableBase<K, V, H>::TryGetValue(const CompatibleKeyType& key, V*& out_pValue) const
{
  nsUInt32 uiIndex = FindEntry(key);
  if (uiIndex != nsInvalidIndex)
  {
    out_pValue = &m_pEntries[uiIndex].value;
    NS_ANALYSIS_ASSUME(out_pValue != nullptr);
    return true;
  }

  return false;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline typename nsHashTableBase<K, V, H>::ConstIterator nsHashTableBase<K, V, H>::Find(const CompatibleKeyType& key) const
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

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline typename nsHashTableBase<K, V, H>::Iterator nsHashTableBase<K, V, H>::Find(const CompatibleKeyType& key)
{
  nsUInt32 uiIndex = FindEntry(key);
  if (uiIndex == nsInvalidIndex)
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
inline const V* nsHashTableBase<K, V, H>::GetValue(const CompatibleKeyType& key) const
{
  nsUInt32 uiIndex = FindEntry(key);
  return (uiIndex != nsInvalidIndex) ? &m_pEntries[uiIndex].value : nullptr;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline V* nsHashTableBase<K, V, H>::GetValue(const CompatibleKeyType& key)
{
  nsUInt32 uiIndex = FindEntry(key);
  return (uiIndex != nsInvalidIndex) ? &m_pEntries[uiIndex].value : nullptr;
}

template <typename K, typename V, typename H>
inline V& nsHashTableBase<K, V, H>::operator[](const K& key)
{
  return FindOrAdd(key, nullptr);
}

template <typename K, typename V, typename H>
V& nsHashTableBase<K, V, H>::FindOrAdd(const K& key, bool* out_pExisted)
{
  const nsUInt32 uiHash = H::Hash(key);
  nsUInt32 uiIndex = FindEntry(uiHash, key);

  if (out_pExisted)
  {
    *out_pExisted = uiIndex != nsInvalidIndex;
  }

  if (uiIndex == nsInvalidIndex)
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
    nsMemoryUtils::CopyConstruct(&m_pEntries[uiIndex].key, key, 1);
    nsMemoryUtils::Construct<ConstructAll>(&m_pEntries[uiIndex].value, 1);
    MarkEntryAsValid(uiIndex);
    ++m_uiCount;
  }

  NS_ASSERT_DEBUG(m_pEntries != nullptr, "Entries should be present");
  return m_pEntries[uiIndex].value;
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
NS_FORCE_INLINE bool nsHashTableBase<K, V, H>::Contains(const CompatibleKeyType& key) const
{
  return FindEntry(key) != nsInvalidIndex;
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE typename nsHashTableBase<K, V, H>::Iterator nsHashTableBase<K, V, H>::GetIterator()
{
  Iterator iterator(*this);
  iterator.SetToBegin();
  return iterator;
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE typename nsHashTableBase<K, V, H>::Iterator nsHashTableBase<K, V, H>::GetEndIterator()
{
  Iterator iterator(*this);
  iterator.SetToEnd();
  return iterator;
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE typename nsHashTableBase<K, V, H>::ConstIterator nsHashTableBase<K, V, H>::GetIterator() const
{
  ConstIterator iterator(*this);
  iterator.SetToBegin();
  return iterator;
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE typename nsHashTableBase<K, V, H>::ConstIterator nsHashTableBase<K, V, H>::GetEndIterator() const
{
  ConstIterator iterator(*this);
  iterator.SetToEnd();
  return iterator;
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE nsAllocator* nsHashTableBase<K, V, H>::GetAllocator() const
{
  return m_pAllocator;
}

template <typename K, typename V, typename H>
nsUInt64 nsHashTableBase<K, V, H>::GetHeapMemoryUsage() const
{
  return ((nsUInt64)m_uiCapacity * sizeof(Entry)) + (sizeof(nsUInt32) * (nsUInt64)GetFlagsCapacity());
}

// private methods
template <typename K, typename V, typename H>
void nsHashTableBase<K, V, H>::SetCapacity(nsUInt32 uiCapacity)
{
  NS_ASSERT_DEBUG(nsMath::IsPowerOf2(uiCapacity), "uiCapacity must be a power of two to avoid modulo during lookup.");
  const nsUInt32 uiOldCapacity = m_uiCapacity;
  m_uiCapacity = uiCapacity;

  Entry* pOldEntries = m_pEntries;
  nsUInt32* pOldEntryFlags = m_pEntryFlags;

  m_pEntries = NS_NEW_RAW_BUFFER(m_pAllocator, Entry, m_uiCapacity);
  m_pEntryFlags = NS_NEW_RAW_BUFFER(m_pAllocator, nsUInt32, GetFlagsCapacity());
  nsMemoryUtils::ZeroFill(m_pEntryFlags, GetFlagsCapacity());

  m_uiCount = 0;
  for (nsUInt32 i = 0; i < uiOldCapacity; ++i)
  {
    if (GetFlags(pOldEntryFlags, i) == VALID_ENTRY)
    {
      NS_VERIFY(!Insert(std::move(pOldEntries[i].key), std::move(pOldEntries[i].value)), "Implementation error");

      nsMemoryUtils::Destruct(&pOldEntries[i].key, 1);
      nsMemoryUtils::Destruct(&pOldEntries[i].value, 1);
    }
  }

  NS_DELETE_RAW_BUFFER(m_pAllocator, pOldEntries);
  NS_DELETE_RAW_BUFFER(m_pAllocator, pOldEntryFlags);
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
NS_ALWAYS_INLINE nsUInt32 nsHashTableBase<K, V, H>::FindEntry(const CompatibleKeyType& key) const
{
  return FindEntry(H::Hash(key), key);
}

template <typename K, typename V, typename H>
template <typename CompatibleKeyType>
inline nsUInt32 nsHashTableBase<K, V, H>::FindEntry(nsUInt32 uiHash, const CompatibleKeyType& key) const
{
  if (m_uiCapacity > 0)
  {
    nsUInt32 uiIndex = uiHash & (m_uiCapacity - 1);
    nsUInt32 uiCounter = 0;
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
  return nsInvalidIndex;
}

#define NS_HASHTABLE_USE_BITFLAGS NS_ON

template <typename K, typename V, typename H>
NS_FORCE_INLINE nsUInt32 nsHashTableBase<K, V, H>::GetFlagsCapacity() const
{
#if NS_ENABLED(NS_HASHTABLE_USE_BITFLAGS)
  return (m_uiCapacity + 15) / 16;
#else
  return m_uiCapacity;
#endif
}

template <typename K, typename V, typename H>
NS_ALWAYS_INLINE nsUInt32 nsHashTableBase<K, V, H>::GetFlags(nsUInt32* pFlags, nsUInt32 uiEntryIndex) const
{
#if NS_ENABLED(NS_HASHTABLE_USE_BITFLAGS)
  const nsUInt32 uiIndex = uiEntryIndex / 16;
  const nsUInt32 uiSubIndex = (uiEntryIndex & 15) * 2;
  return (pFlags[uiIndex] >> uiSubIndex) & FLAGS_MASK;
#else
  return pFlags[uiEntryIndex] & FLAGS_MASK;
#endif
}

template <typename K, typename V, typename H>
void nsHashTableBase<K, V, H>::SetFlags(nsUInt32 uiEntryIndex, nsUInt32 uiFlags)
{
#if NS_ENABLED(NS_HASHTABLE_USE_BITFLAGS)
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

template <typename K, typename V, typename H>
NS_FORCE_INLINE bool nsHashTableBase<K, V, H>::IsFreeEntry(nsUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == FREE_ENTRY;
}

template <typename K, typename V, typename H>
NS_FORCE_INLINE bool nsHashTableBase<K, V, H>::IsValidEntry(nsUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == VALID_ENTRY;
}

template <typename K, typename V, typename H>
NS_FORCE_INLINE bool nsHashTableBase<K, V, H>::IsDeletedEntry(nsUInt32 uiEntryIndex) const
{
  return GetFlags(m_pEntryFlags, uiEntryIndex) == DELETED_ENTRY;
}

template <typename K, typename V, typename H>
NS_FORCE_INLINE void nsHashTableBase<K, V, H>::MarkEntryAsFree(nsUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, FREE_ENTRY);
}

template <typename K, typename V, typename H>
NS_FORCE_INLINE void nsHashTableBase<K, V, H>::MarkEntryAsValid(nsUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, VALID_ENTRY);
}

template <typename K, typename V, typename H>
NS_FORCE_INLINE void nsHashTableBase<K, V, H>::MarkEntryAsDeleted(nsUInt32 uiEntryIndex)
{
  SetFlags(uiEntryIndex, DELETED_ENTRY);
}


template <typename K, typename V, typename H, typename A>
nsHashTable<K, V, H, A>::nsHashTable()
  : nsHashTableBase<K, V, H>(A::GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
nsHashTable<K, V, H, A>::nsHashTable(nsAllocator* pAllocator)
  : nsHashTableBase<K, V, H>(pAllocator)
{
}

template <typename K, typename V, typename H, typename A>
nsHashTable<K, V, H, A>::nsHashTable(const nsHashTable<K, V, H, A>& other)
  : nsHashTableBase<K, V, H>(other, A::GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
nsHashTable<K, V, H, A>::nsHashTable(const nsHashTableBase<K, V, H>& other)
  : nsHashTableBase<K, V, H>(other, A::GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
nsHashTable<K, V, H, A>::nsHashTable(nsHashTable<K, V, H, A>&& other)
  : nsHashTableBase<K, V, H>(std::move(other), other.GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
nsHashTable<K, V, H, A>::nsHashTable(nsHashTableBase<K, V, H>&& other)
  : nsHashTableBase<K, V, H>(std::move(other), other.GetAllocator())
{
}

template <typename K, typename V, typename H, typename A>
void nsHashTable<K, V, H, A>::operator=(const nsHashTable<K, V, H, A>& rhs)
{
  nsHashTableBase<K, V, H>::operator=(rhs);
}

template <typename K, typename V, typename H, typename A>
void nsHashTable<K, V, H, A>::operator=(const nsHashTableBase<K, V, H>& rhs)
{
  nsHashTableBase<K, V, H>::operator=(rhs);
}

template <typename K, typename V, typename H, typename A>
void nsHashTable<K, V, H, A>::operator=(nsHashTable<K, V, H, A>&& rhs)
{
  nsHashTableBase<K, V, H>::operator=(std::move(rhs));
}

template <typename K, typename V, typename H, typename A>
void nsHashTable<K, V, H, A>::operator=(nsHashTableBase<K, V, H>&& rhs)
{
  nsHashTableBase<K, V, H>::operator=(std::move(rhs));
}

template <typename KeyType, typename ValueType, typename Hasher>
void nsHashTableBase<KeyType, ValueType, Hasher>::Swap(nsHashTableBase<KeyType, ValueType, Hasher>& other)
{
  nsMath::Swap(this->m_pEntries, other.m_pEntries);
  nsMath::Swap(this->m_pEntryFlags, other.m_pEntryFlags);
  nsMath::Swap(this->m_uiCount, other.m_uiCount);
  nsMath::Swap(this->m_uiCapacity, other.m_uiCapacity);
  nsMath::Swap(this->m_pAllocator, other.m_pAllocator);
}
