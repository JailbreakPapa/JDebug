#pragma once

template <class Container>
NS_ALWAYS_INLINE nsUInt32 nsBitfield<Container>::GetBitInt(nsUInt32 uiBitIndex) const
{
  return (uiBitIndex >> 5); // div 32
}

template <class Container>
NS_ALWAYS_INLINE nsUInt32 nsBitfield<Container>::GetBitMask(nsUInt32 uiBitIndex) const
{
  return 1 << (uiBitIndex & 0x1F); // modulo 32, shifted to bit position
}

template <class Container>
NS_ALWAYS_INLINE nsUInt32 nsBitfield<Container>::GetCount() const
{
  return m_uiCount;
}

template <class Container>
template <typename> // Second template needed so that the compiler only instantiates it when called. Needed to prevent errors with containers that do not support this.
void nsBitfield<Container>::SetCountUninitialized(nsUInt32 uiBitCount)
{
  const nsUInt32 uiInts = (uiBitCount + 31) >> 5;
  m_Container.SetCountUninitialized(uiInts);

  m_uiCount = uiBitCount;
}

template <class Container>
void nsBitfield<Container>::SetCount(nsUInt32 uiBitCount, bool bSetNew)
{
  if (m_uiCount == uiBitCount)
    return;

  const nsUInt32 uiOldBits = m_uiCount;

  SetCountUninitialized(uiBitCount);

  // if there are new bits, initialize them
  if (uiBitCount > uiOldBits)
  {
    if (bSetNew)
      SetBitRange(uiOldBits, uiBitCount - uiOldBits);
    else
      ClearBitRange(uiOldBits, uiBitCount - uiOldBits);
  }
}

template <class Container>
NS_ALWAYS_INLINE bool nsBitfield<Container>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <class Container>
bool nsBitfield<Container>::IsAnyBitSet(nsUInt32 uiFirstBit /*= 0*/, nsUInt32 uiNumBits /*= 0xFFFFFFFF*/) const
{
  if (m_uiCount == 0 || uiNumBits == 0)
    return false;

  NS_ASSERT_DEBUG(uiFirstBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiFirstBit, m_uiCount);

  const nsUInt32 uiLastBit = nsMath::Min<nsUInt32>(uiFirstBit + uiNumBits, m_uiCount - 1);

  const nsUInt32 uiFirstInt = GetBitInt(uiFirstBit);
  const nsUInt32 uiLastInt = GetBitInt(uiLastBit);

  // all within the same int
  if (uiFirstInt == uiLastInt)
  {
    for (nsUInt32 i = uiFirstBit; i <= uiLastBit; ++i)
    {
      if (IsBitSet(i))
        return true;
    }
  }
  else
  {
    const nsUInt32 uiNextIntBit = (uiFirstInt + 1) * 32;
    const nsUInt32 uiPrevIntBit = uiLastInt * 32;

    // check the bits in the first int individually
    for (nsUInt32 i = uiFirstBit; i < uiNextIntBit; ++i)
    {
      if (IsBitSet(i))
        return true;
    }

    // check the bits in the ints in between with one operation
    for (nsUInt32 i = uiFirstInt + 1; i < uiLastInt; ++i)
    {
      if ((m_Container[i] & 0xFFFFFFFF) != 0)
        return true;
    }

    // check the bits in the last int individually
    for (nsUInt32 i = uiPrevIntBit; i <= uiLastBit; ++i)
    {
      if (IsBitSet(i))
        return true;
    }
  }

  return false;
}

template <class Container>
NS_ALWAYS_INLINE bool nsBitfield<Container>::IsNoBitSet(nsUInt32 uiFirstBit /*= 0*/, nsUInt32 uiLastBit /*= 0xFFFFFFFF*/) const
{
  return !IsAnyBitSet(uiFirstBit, uiLastBit);
}

template <class Container>
bool nsBitfield<Container>::AreAllBitsSet(nsUInt32 uiFirstBit /*= 0*/, nsUInt32 uiNumBits /*= 0xFFFFFFFF*/) const
{
  if (m_uiCount == 0 || uiNumBits == 0)
    return false;

  NS_ASSERT_DEBUG(uiFirstBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiFirstBit, m_uiCount);

  const nsUInt32 uiLastBit = nsMath::Min<nsUInt32>(uiFirstBit + uiNumBits, m_uiCount - 1);

  const nsUInt32 uiFirstInt = GetBitInt(uiFirstBit);
  const nsUInt32 uiLastInt = GetBitInt(uiLastBit);

  // all within the same int
  if (uiFirstInt == uiLastInt)
  {
    for (nsUInt32 i = uiFirstBit; i <= uiLastBit; ++i)
    {
      if (!IsBitSet(i))
        return false;
    }
  }
  else
  {
    const nsUInt32 uiNextIntBit = (uiFirstInt + 1) * 32;
    const nsUInt32 uiPrevIntBit = uiLastInt * 32;

    // check the bits in the first int individually
    for (nsUInt32 i = uiFirstBit; i < uiNextIntBit; ++i)
    {
      if (!IsBitSet(i))
        return false;
    }

    // check the bits in the ints in between with one operation
    for (nsUInt32 i = uiFirstInt + 1; i < uiLastInt; ++i)
    {
      if (m_Container[i] != 0xFFFFFFFF)
        return false;
    }

    // check the bits in the last int individually
    for (nsUInt32 i = uiPrevIntBit; i <= uiLastBit; ++i)
    {
      if (!IsBitSet(i))
        return false;
    }
  }

  return true;
}

template <class Container>
NS_ALWAYS_INLINE void nsBitfield<Container>::Clear()
{
  m_uiCount = 0;
  m_Container.Clear();
}

template <class Container>
void nsBitfield<Container>::SetBit(nsUInt32 uiBit)
{
  NS_ASSERT_DEBUG(uiBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, m_uiCount);

  m_Container[GetBitInt(uiBit)] |= GetBitMask(uiBit);
}

template <class Container>
void nsBitfield<Container>::ClearBit(nsUInt32 uiBit)
{
  NS_ASSERT_DEBUG(uiBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, m_uiCount);

  m_Container[GetBitInt(uiBit)] &= ~GetBitMask(uiBit);
}

template <class Container>
NS_ALWAYS_INLINE void nsBitfield<Container>::SetBitValue(nsUInt32 uiBit, bool bValue)
{
  if (bValue)
  {
    SetBit(uiBit);
  }
  else
  {
    ClearBit(uiBit);
  }
}

template <class Container>
bool nsBitfield<Container>::IsBitSet(nsUInt32 uiBit) const
{
  NS_ASSERT_DEBUG(uiBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, m_uiCount);

  return (m_Container[GetBitInt(uiBit)] & GetBitMask(uiBit)) != 0;
}

template <class Container>
void nsBitfield<Container>::ClearAllBits()
{
  for (nsUInt32 i = 0; i < m_Container.GetCount(); ++i)
    m_Container[i] = 0;
}

template <class Container>
void nsBitfield<Container>::SetAllBits()
{
  for (nsUInt32 i = 0; i < m_Container.GetCount(); ++i)
    m_Container[i] = 0xFFFFFFFF;
}

template <class Container>
void nsBitfield<Container>::SetBitRange(nsUInt32 uiFirstBit, nsUInt32 uiNumBits)
{
  if (m_uiCount == 0 || uiNumBits == 0)
    return;

  NS_ASSERT_DEBUG(uiFirstBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiFirstBit, m_uiCount);

  const nsUInt32 uiLastBit = uiFirstBit + uiNumBits - 1;

  const nsUInt32 uiFirstInt = GetBitInt(uiFirstBit);
  const nsUInt32 uiLastInt = GetBitInt(uiLastBit);

  // all within the same int
  if (uiFirstInt == uiLastInt)
  {
    for (nsUInt32 i = uiFirstBit; i <= uiLastBit; ++i)
      SetBit(i);

    return;
  }

  const nsUInt32 uiNextIntBit = (uiFirstInt + 1) * 32;
  const nsUInt32 uiPrevIntBit = uiLastInt * 32;

  // set the bits in the first int individually
  for (nsUInt32 i = uiFirstBit; i < uiNextIntBit; ++i)
    SetBit(i);

  // set the bits in the ints in between with one operation
  for (nsUInt32 i = uiFirstInt + 1; i < uiLastInt; ++i)
    m_Container[i] = 0xFFFFFFFF;

  // set the bits in the last int individually
  for (nsUInt32 i = uiPrevIntBit; i <= uiLastBit; ++i)
    SetBit(i);
}

template <class Container>
void nsBitfield<Container>::ClearBitRange(nsUInt32 uiFirstBit, nsUInt32 uiNumBits)
{
  if (m_uiCount == 0 || uiNumBits == 0)
    return;

  NS_ASSERT_DEBUG(uiFirstBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiFirstBit, m_uiCount);

  const nsUInt32 uiLastBit = uiFirstBit + uiNumBits - 1;

  const nsUInt32 uiFirstInt = GetBitInt(uiFirstBit);
  const nsUInt32 uiLastInt = GetBitInt(uiLastBit);

  // all within the same int
  if (uiFirstInt == uiLastInt)
  {
    for (nsUInt32 i = uiFirstBit; i <= uiLastBit; ++i)
      ClearBit(i);

    return;
  }

  const nsUInt32 uiNextIntBit = (uiFirstInt + 1) * 32;
  const nsUInt32 uiPrevIntBit = uiLastInt * 32;

  // set the bits in the first int individually
  for (nsUInt32 i = uiFirstBit; i < uiNextIntBit; ++i)
    ClearBit(i);

  // set the bits in the ints in between with one operation
  for (nsUInt32 i = uiFirstInt + 1; i < uiLastInt; ++i)
    m_Container[i] = 0;

  // set the bits in the last int individually
  for (nsUInt32 i = uiPrevIntBit; i <= uiLastBit; ++i)
    ClearBit(i);
}

template <class Container>
void nsBitfield<Container>::Swap(nsBitfield<Container>& other)
{
  nsMath::Swap(m_uiCount, other.m_uiCount);
  m_Container.Swap(other.m_Container);
}

template <class Container>
NS_ALWAYS_INLINE typename nsBitfield<Container>::ConstIterator nsBitfield<Container>::GetIterator() const
{
  return ConstIterator(*this);
};

template <class Container>
NS_ALWAYS_INLINE typename nsBitfield<Container>::ConstIterator nsBitfield<Container>::GetEndIterator() const
{
  return ConstIterator();
};

//////////////////////////////////////////////////////////////////////////
// nsBitfield<Container>::ConstIterator

template <class Container>
nsBitfield<Container>::ConstIterator::ConstIterator(const nsBitfield<Container>& bitfield)
{
  m_pBitfield = &bitfield;
  FindNextChunk(0);
}

template <class Container>
NS_ALWAYS_INLINE bool nsBitfield<Container>::ConstIterator::IsValid() const
{
  return m_pBitfield != nullptr;
}

template <class Container>
NS_ALWAYS_INLINE nsUInt32 nsBitfield<Container>::ConstIterator::Value() const
{
  return *m_Iterator + (m_uiChunk << 5);
}

template <class Container>
NS_ALWAYS_INLINE void nsBitfield<Container>::ConstIterator::Next()
{
  ++m_Iterator;
  if (!m_Iterator.IsValid())
  {
    FindNextChunk(m_uiChunk + 1);
  }
}

template <class Container>
NS_ALWAYS_INLINE bool nsBitfield<Container>::ConstIterator::operator==(const ConstIterator& other) const
{
  return m_pBitfield == other.m_pBitfield && m_Iterator == other.m_Iterator && m_uiChunk == other.m_uiChunk;
}

template <class Container>
NS_ALWAYS_INLINE bool nsBitfield<Container>::ConstIterator::operator!=(const ConstIterator& other) const
{
  return m_pBitfield != other.m_pBitfield || m_Iterator != other.m_Iterator || m_uiChunk != other.m_uiChunk;
}

template <class Container>
NS_ALWAYS_INLINE nsUInt32 nsBitfield<Container>::ConstIterator::operator*() const
{
  return Value();
}

template <class Container>
NS_ALWAYS_INLINE void nsBitfield<Container>::ConstIterator::operator++()
{
  Next();
}

template <class Container>
void nsBitfield<Container>::ConstIterator::FindNextChunk(nsUInt32 uiStartChunk)
{
  if (uiStartChunk < m_pBitfield->m_Container.GetCount())
  {
    const nsUInt32 uiLastChunk = m_pBitfield->m_Container.GetCount() - 1;
    for (nsUInt32 i = uiStartChunk; i < uiLastChunk; ++i)
    {
      if (m_pBitfield->m_Container[i] != 0)
      {
        m_uiChunk = i;
        m_Iterator = sub_iterator(m_pBitfield->m_Container[i]);
        return;
      }
    }

    const nsUInt32 uiMask = 0xFFFFFFFF >> (32 - (m_pBitfield->m_uiCount - (uiLastChunk << 5)));
    if ((m_pBitfield->m_Container[uiLastChunk] & uiMask) != 0)
    {
      m_uiChunk = uiLastChunk;
      m_Iterator = sub_iterator(m_pBitfield->m_Container[uiLastChunk] & uiMask);
      return;
    }
  }

  // End iterator.
  m_pBitfield = nullptr;
  m_uiChunk = 0;
  m_Iterator = sub_iterator();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename T>
NS_ALWAYS_INLINE nsStaticBitfield<T>::nsStaticBitfield()
{
  static_assert(std::is_unsigned<T>::value, "Storage type must be unsigned");
}

template <typename T>
NS_ALWAYS_INLINE nsStaticBitfield<T> nsStaticBitfield<T>::MakeFromMask(StorageType bits)
{
  return nsStaticBitfield<T>(bits);
}

template <typename T>
NS_ALWAYS_INLINE bool nsStaticBitfield<T>::IsAnyBitSet() const
{
  return m_Storage != 0;
}

template <typename T>
NS_ALWAYS_INLINE bool nsStaticBitfield<T>::IsNoBitSet() const
{
  return m_Storage == 0;
}

template <typename T>
bool nsStaticBitfield<T>::AreAllBitsSet() const
{
  const T inv = ~m_Storage;
  return inv == 0;
}

template <typename T>
void nsStaticBitfield<T>::ClearBitRange(nsUInt32 uiFirstBit, nsUInt32 uiNumBits)
{
  NS_ASSERT_DEBUG(uiFirstBit < GetStorageTypeBitCount(), "Cannot access first bit {0}, the bitfield only has {1} bits.", uiFirstBit, GetStorageTypeBitCount());

  T mask = (uiNumBits / 8 >= sizeof(T)) ? (~static_cast<T>(0)) : ((static_cast<T>(1) << uiNumBits) - 1);
  mask <<= uiFirstBit;
  mask = ~mask;
  m_Storage &= mask;
}

template <typename T>
void nsStaticBitfield<T>::SetBitRange(nsUInt32 uiFirstBit, nsUInt32 uiNumBits)
{
  NS_ASSERT_DEBUG(uiFirstBit < GetStorageTypeBitCount(), "Cannot access first bit {0}, the bitfield only has {1} bits.", uiFirstBit, GetStorageTypeBitCount());

  T mask = (uiNumBits / 8 >= sizeof(T)) ? (~static_cast<T>(0)) : ((static_cast<T>(1) << uiNumBits) - 1);
  mask <<= uiFirstBit;
  m_Storage |= mask;
}

template <typename T>
NS_ALWAYS_INLINE nsUInt32 nsStaticBitfield<T>::GetNumBitsSet() const
{
  return nsMath::CountBits(m_Storage);
}

template <typename T>
NS_ALWAYS_INLINE nsUInt32 nsStaticBitfield<T>::GetHighestBitSet() const
{
  return m_Storage == 0 ? GetStorageTypeBitCount() : nsMath::FirstBitHigh(m_Storage);
}

template <typename T>
NS_ALWAYS_INLINE nsUInt32 nsStaticBitfield<T>::GetLowestBitSet() const
{
  return m_Storage == 0 ? GetStorageTypeBitCount() : nsMath::FirstBitLow(m_Storage);
}

template <typename T>
NS_ALWAYS_INLINE void nsStaticBitfield<T>::SetAllBits()
{
  m_Storage = nsMath::MaxValue<T>(); // possible because we assert that T is unsigned
}

template <typename T>
NS_ALWAYS_INLINE void nsStaticBitfield<T>::ClearAllBits()
{
  m_Storage = 0;
}

template <typename T>
NS_ALWAYS_INLINE bool nsStaticBitfield<T>::IsBitSet(nsUInt32 uiBit) const
{
  NS_ASSERT_DEBUG(uiBit < GetStorageTypeBitCount(), "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, GetStorageTypeBitCount());

  return (m_Storage & (static_cast<T>(1u) << uiBit)) != 0;
}

template <typename T>
NS_ALWAYS_INLINE void nsStaticBitfield<T>::ClearBit(nsUInt32 uiBit)
{
  NS_ASSERT_DEBUG(uiBit < GetStorageTypeBitCount(), "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, GetStorageTypeBitCount());

  m_Storage &= ~(static_cast<T>(1u) << uiBit);
}

template <typename T>
NS_ALWAYS_INLINE void nsStaticBitfield<T>::SetBitValue(nsUInt32 uiBit, bool bValue)
{
  if (bValue)
  {
    SetBit(uiBit);
  }
  else
  {
    ClearBit(uiBit);
  }
}

template <typename T>
NS_ALWAYS_INLINE void nsStaticBitfield<T>::SetBit(nsUInt32 uiBit)
{
  NS_ASSERT_DEBUG(uiBit < GetStorageTypeBitCount(), "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, GetStorageTypeBitCount());

  m_Storage |= static_cast<T>(1u) << uiBit;
}

template <typename T>
NS_ALWAYS_INLINE void nsStaticBitfield<T>::SetValue(T value)
{
  m_Storage = value;
}

template <typename T>
NS_ALWAYS_INLINE T nsStaticBitfield<T>::GetValue() const
{
  return m_Storage;
}

template <typename T>
NS_ALWAYS_INLINE void nsStaticBitfield<T>::Swap(nsStaticBitfield<T>& other)
{
  nsMath::Swap(m_Storage, other.m_Storage);
}
