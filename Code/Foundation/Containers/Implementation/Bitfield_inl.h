#pragma once

template <class Container>
WD_ALWAYS_INLINE wdUInt32 wdBitfield<Container>::GetBitInt(wdUInt32 uiBitIndex) const
{
  return (uiBitIndex >> 5); // div 32
}

template <class Container>
WD_ALWAYS_INLINE wdUInt32 wdBitfield<Container>::GetBitMask(wdUInt32 uiBitIndex) const
{
  return 1 << (uiBitIndex & 0x1F); // modulo 32, shifted to bit position
}

template <class Container>
WD_ALWAYS_INLINE wdUInt32 wdBitfield<Container>::GetCount() const
{
  return m_uiCount;
}

template <class Container>
template <typename> // Second template needed so that the compiler only instantiates it when called. Needed to prevent errors with containers that do
                    // not support this.
void wdBitfield<Container>::SetCountUninitialized(wdUInt32 uiBitCount)
{
  const wdUInt32 uiInts = (uiBitCount + 31) >> 5;
  m_Container.SetCountUninitialized(uiInts);

  m_uiCount = uiBitCount;
}

template <class Container>
void wdBitfield<Container>::SetCount(wdUInt32 uiBitCount, bool bSetNew)
{
  if (m_uiCount == uiBitCount)
    return;

  const wdUInt32 uiOldBits = m_uiCount;

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
WD_ALWAYS_INLINE bool wdBitfield<Container>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <class Container>
bool wdBitfield<Container>::IsAnyBitSet(wdUInt32 uiFirstBit /*= 0*/, wdUInt32 uiNumBits /*= 0xFFFFFFFF*/) const
{
  if (m_uiCount == 0 || uiNumBits == 0)
    return false;

  WD_ASSERT_DEBUG(uiFirstBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiFirstBit, m_uiCount);

  const wdUInt32 uiLastBit = wdMath::Min<wdUInt32>(uiFirstBit + uiNumBits, m_uiCount - 1);

  const wdUInt32 uiFirstInt = GetBitInt(uiFirstBit);
  const wdUInt32 uiLastInt = GetBitInt(uiLastBit);

  // all within the same int
  if (uiFirstInt == uiLastInt)
  {
    for (wdUInt32 i = uiFirstBit; i <= uiLastBit; ++i)
    {
      if (IsBitSet(i))
        return true;
    }
  }
  else
  {
    const wdUInt32 uiNextIntBit = (uiFirstInt + 1) * 32;
    const wdUInt32 uiPrevIntBit = uiLastInt * 32;

    // check the bits in the first int individually
    for (wdUInt32 i = uiFirstBit; i < uiNextIntBit; ++i)
    {
      if (IsBitSet(i))
        return true;
    }

    // check the bits in the ints in between with one operation
    for (wdUInt32 i = uiFirstInt + 1; i < uiLastInt; ++i)
    {
      if ((m_Container[i] & 0xFFFFFFFF) != 0)
        return true;
    }

    // check the bits in the last int individually
    for (wdUInt32 i = uiPrevIntBit; i <= uiLastBit; ++i)
    {
      if (IsBitSet(i))
        return true;
    }
  }

  return false;
}

template <class Container>
WD_ALWAYS_INLINE bool wdBitfield<Container>::IsNoBitSet(wdUInt32 uiFirstBit /*= 0*/, wdUInt32 uiLastBit /*= 0xFFFFFFFF*/) const
{
  return !IsAnyBitSet(uiFirstBit, uiLastBit);
}

template <class Container>
bool wdBitfield<Container>::AreAllBitsSet(wdUInt32 uiFirstBit /*= 0*/, wdUInt32 uiNumBits /*= 0xFFFFFFFF*/) const
{
  if (m_uiCount == 0 || uiNumBits == 0)
    return false;

  WD_ASSERT_DEBUG(uiFirstBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiFirstBit, m_uiCount);

  const wdUInt32 uiLastBit = wdMath::Min<wdUInt32>(uiFirstBit + uiNumBits, m_uiCount - 1);

  const wdUInt32 uiFirstInt = GetBitInt(uiFirstBit);
  const wdUInt32 uiLastInt = GetBitInt(uiLastBit);

  // all within the same int
  if (uiFirstInt == uiLastInt)
  {
    for (wdUInt32 i = uiFirstBit; i <= uiLastBit; ++i)
    {
      if (!IsBitSet(i))
        return false;
    }
  }
  else
  {
    const wdUInt32 uiNextIntBit = (uiFirstInt + 1) * 32;
    const wdUInt32 uiPrevIntBit = uiLastInt * 32;

    // check the bits in the first int individually
    for (wdUInt32 i = uiFirstBit; i < uiNextIntBit; ++i)
    {
      if (!IsBitSet(i))
        return false;
    }

    // check the bits in the ints in between with one operation
    for (wdUInt32 i = uiFirstInt + 1; i < uiLastInt; ++i)
    {
      if (m_Container[i] != 0xFFFFFFFF)
        return false;
    }

    // check the bits in the last int individually
    for (wdUInt32 i = uiPrevIntBit; i <= uiLastBit; ++i)
    {
      if (!IsBitSet(i))
        return false;
    }
  }

  return true;
}

template <class Container>
WD_ALWAYS_INLINE void wdBitfield<Container>::Clear()
{
  m_uiCount = 0;
  m_Container.Clear();
}

template <class Container>
void wdBitfield<Container>::SetBit(wdUInt32 uiBit)
{
  WD_ASSERT_DEBUG(uiBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, m_uiCount);

  m_Container[GetBitInt(uiBit)] |= GetBitMask(uiBit);
}

template <class Container>
void wdBitfield<Container>::ClearBit(wdUInt32 uiBit)
{
  WD_ASSERT_DEBUG(uiBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, m_uiCount);

  m_Container[GetBitInt(uiBit)] &= ~GetBitMask(uiBit);
}

template <class Container>
bool wdBitfield<Container>::IsBitSet(wdUInt32 uiBit) const
{
  WD_ASSERT_DEBUG(uiBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, m_uiCount);

  return (m_Container[GetBitInt(uiBit)] & GetBitMask(uiBit)) != 0;
}

template <class Container>
void wdBitfield<Container>::ClearAllBits()
{
  for (wdUInt32 i = 0; i < m_Container.GetCount(); ++i)
    m_Container[i] = 0;
}

template <class Container>
void wdBitfield<Container>::SetAllBits()
{
  for (wdUInt32 i = 0; i < m_Container.GetCount(); ++i)
    m_Container[i] = 0xFFFFFFFF;
}

template <class Container>
void wdBitfield<Container>::SetBitRange(wdUInt32 uiFirstBit, wdUInt32 uiNumBits)
{
  if (m_uiCount == 0 || uiNumBits == 0)
    return;

  WD_ASSERT_DEBUG(uiFirstBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiFirstBit, m_uiCount);

  const wdUInt32 uiLastBit = uiFirstBit + uiNumBits - 1;

  const wdUInt32 uiFirstInt = GetBitInt(uiFirstBit);
  const wdUInt32 uiLastInt = GetBitInt(uiLastBit);

  // all within the same int
  if (uiFirstInt == uiLastInt)
  {
    for (wdUInt32 i = uiFirstBit; i <= uiLastBit; ++i)
      SetBit(i);

    return;
  }

  const wdUInt32 uiNextIntBit = (uiFirstInt + 1) * 32;
  const wdUInt32 uiPrevIntBit = uiLastInt * 32;

  // set the bits in the first int individually
  for (wdUInt32 i = uiFirstBit; i < uiNextIntBit; ++i)
    SetBit(i);

  // set the bits in the ints in between with one operation
  for (wdUInt32 i = uiFirstInt + 1; i < uiLastInt; ++i)
    m_Container[i] = 0xFFFFFFFF;

  // set the bits in the last int individually
  for (wdUInt32 i = uiPrevIntBit; i <= uiLastBit; ++i)
    SetBit(i);
}

template <class Container>
void wdBitfield<Container>::ClearBitRange(wdUInt32 uiFirstBit, wdUInt32 uiNumBits)
{
  if (m_uiCount == 0 || uiNumBits == 0)
    return;

  WD_ASSERT_DEBUG(uiFirstBit < m_uiCount, "Cannot access bit {0}, the bitfield only has {1} bits.", uiFirstBit, m_uiCount);

  const wdUInt32 uiLastBit = uiFirstBit + uiNumBits - 1;

  const wdUInt32 uiFirstInt = GetBitInt(uiFirstBit);
  const wdUInt32 uiLastInt = GetBitInt(uiLastBit);

  // all within the same int
  if (uiFirstInt == uiLastInt)
  {
    for (wdUInt32 i = uiFirstBit; i <= uiLastBit; ++i)
      ClearBit(i);

    return;
  }

  const wdUInt32 uiNextIntBit = (uiFirstInt + 1) * 32;
  const wdUInt32 uiPrevIntBit = uiLastInt * 32;

  // set the bits in the first int individually
  for (wdUInt32 i = uiFirstBit; i < uiNextIntBit; ++i)
    ClearBit(i);

  // set the bits in the ints in between with one operation
  for (wdUInt32 i = uiFirstInt + 1; i < uiLastInt; ++i)
    m_Container[i] = 0;

  // set the bits in the last int individually
  for (wdUInt32 i = uiPrevIntBit; i <= uiLastBit; ++i)
    ClearBit(i);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename T>
WD_ALWAYS_INLINE wdStaticBitfield<T>::wdStaticBitfield()
{
  static_assert(std::is_unsigned<T>::value, "Storage type must be unsigned");
}

template <typename T>
WD_ALWAYS_INLINE wdStaticBitfield<T> wdStaticBitfield<T>::FromMask(StorageType bits)
{
  return wdStaticBitfield<T>(bits);
}

template <typename T>
WD_ALWAYS_INLINE bool wdStaticBitfield<T>::IsAnyBitSet() const
{
  return m_Storage != 0;
}

template <typename T>
WD_ALWAYS_INLINE bool wdStaticBitfield<T>::IsNoBitSet() const
{
  return m_Storage == 0;
}

template <typename T>
bool wdStaticBitfield<T>::AreAllBitsSet() const
{
  const T inv = ~m_Storage;
  return inv == 0;
}

template <typename T>
void wdStaticBitfield<T>::ClearBitRange(wdUInt32 uiFirstBit, wdUInt32 uiNumBits)
{
  WD_ASSERT_DEBUG(uiFirstBit < GetNumBits(), "Cannot access first bit {0}, the bitfield only has {1} bits.", uiFirstBit, GetNumBits());

  for (wdUInt32 i = 0; i < uiNumBits; ++i)
  {
    const wdUInt32 uiBit = uiFirstBit + i;
    m_Storage &= ~(static_cast<T>(1u) << uiBit);
  }
}

template <typename T>
void wdStaticBitfield<T>::SetBitRange(wdUInt32 uiFirstBit, wdUInt32 uiNumBits)
{
  WD_ASSERT_DEBUG(uiFirstBit < GetNumBits(), "Cannot access first bit {0}, the bitfield only has {1} bits.", uiFirstBit, GetNumBits());

  for (wdUInt32 i = 0; i < uiNumBits; ++i)
  {
    const wdUInt32 uiBit = uiFirstBit + i;
    m_Storage |= static_cast<T>(1u) << uiBit;
  }
}

template <typename T>
WD_ALWAYS_INLINE void wdStaticBitfield<T>::SetAllBits()
{
  m_Storage = wdMath::MaxValue<T>(); // possible because we assert that T is unsigned
}

template <typename T>
WD_ALWAYS_INLINE void wdStaticBitfield<T>::ClearAllBits()
{
  m_Storage = 0;
}

template <typename T>
WD_ALWAYS_INLINE bool wdStaticBitfield<T>::IsBitSet(wdUInt32 uiBit) const
{
  WD_ASSERT_DEBUG(uiBit < GetNumBits(), "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, GetNumBits());

  return (m_Storage & (static_cast<T>(1u) << uiBit)) != 0;
}

template <typename T>
WD_ALWAYS_INLINE void wdStaticBitfield<T>::ClearBit(wdUInt32 uiBit)
{
  WD_ASSERT_DEBUG(uiBit < GetNumBits(), "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, GetNumBits());

  m_Storage &= ~(static_cast<T>(1u) << uiBit);
}

template <typename T>
WD_ALWAYS_INLINE void wdStaticBitfield<T>::SetBit(wdUInt32 uiBit)
{
  WD_ASSERT_DEBUG(uiBit < GetNumBits(), "Cannot access bit {0}, the bitfield only has {1} bits.", uiBit, GetNumBits());

  m_Storage |= static_cast<T>(1u) << uiBit;
}

template <typename T>
WD_ALWAYS_INLINE void wdStaticBitfield<T>::SetValue(T value)
{
  m_Storage = value;
}

template <typename T>
WD_ALWAYS_INLINE T wdStaticBitfield<T>::GetValue() const
{
  return m_Storage;
}
