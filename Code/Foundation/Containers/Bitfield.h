#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Math/Constants.h>

/// \brief A template interface, that turns any array class into a bitfield.
///
/// This class provides an interface to work with single bits, to store true/false values.
/// The underlying container is configurable, though it must support random access and a 'SetCount' function and it must use elements of type
/// wdUInt32. In most cases a dynamic array should be used. For this case the wdDynamicBitfield typedef is already available. There is also an
/// wdHybridBitfield typedef.
template <class Container>
class wdBitfield
{
public:
  wdBitfield() = default;

  /// \brief Returns the number of bits that this bitfield stores.
  wdUInt32 GetCount() const; // [tested]

  /// \brief Resizes the Bitfield to hold the given number of bits. This version does NOT initialize new bits!
  template <typename = void>                       // Template is used to only conditionally compile this function in when it is actually used.
  void SetCountUninitialized(wdUInt32 uiBitCount); // [tested]

  /// \brief Resizes the Bitfield to hold the given number of bits. If \a bSetNew is true, new bits are set to 1, otherwise they are cleared to 0.
  void SetCount(wdUInt32 uiBitCount, bool bSetNew = false); // [tested]

  /// \brief Returns true, if the bitfield does not store any bits.
  bool IsEmpty() const; // [tested]

  /// \brief Returns true, if the bitfield is not empty and any bit is 1.
  bool IsAnyBitSet(wdUInt32 uiFirstBit = 0, wdUInt32 uiNumBits = 0xFFFFFFFF) const; // [tested]

  /// \brief Returns true, if the bitfield is empty or all bits are set to zero.
  bool IsNoBitSet(wdUInt32 uiFirstBit = 0, wdUInt32 uiNumBits = 0xFFFFFFFF) const; // [tested]

  /// \brief Returns true, if the bitfield is not empty and all bits are set to one.
  bool AreAllBitsSet(wdUInt32 uiFirstBit = 0, wdUInt32 uiNumBits = 0xFFFFFFFF) const; // [tested]

  /// \brief Discards all bits and sets count to zero.
  void Clear(); // [tested]

  /// \brief Sets the given bit to 1.
  void SetBit(wdUInt32 uiBit); // [tested]

  /// \brief Clears the given bit to 0.
  void ClearBit(wdUInt32 uiBit); // [tested]

  /// \brief Returns true, if the given bit is set to 1.
  bool IsBitSet(wdUInt32 uiBit) const; // [tested]

  /// \brief Clears all bits to 0.
  void ClearAllBits(); // [tested]

  /// \brief Sets all bits to 1.
  void SetAllBits(); // [tested]

  /// \brief Sets the range starting at uiFirstBit up to (and including) uiLastBit to 1.
  void SetBitRange(wdUInt32 uiFirstBit, wdUInt32 uiNumBits); // [tested]

  /// \brief Clears the range starting at uiFirstBit up to (and including) uiLastBit to 0.
  void ClearBitRange(wdUInt32 uiFirstBit, wdUInt32 uiNumBits); // [tested]

private:
  wdUInt32 GetBitInt(wdUInt32 uiBitIndex) const;
  wdUInt32 GetBitMask(wdUInt32 uiBitIndex) const;

  wdUInt32 m_uiCount = 0;
  Container m_Container;
};

/// \brief This should be the main type of bitfield to use, although other internal container types are possible.
using wdDynamicBitfield = wdBitfield<wdDynamicArray<wdUInt32>>;

/// \brief An wdBitfield that uses a hybrid array as internal container.
template <wdUInt32 BITS>
using wdHybridBitfield = wdBitfield<wdHybridArray<wdUInt32, (BITS + 31) / 32>>;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename T>
class wdStaticBitfield
{
public:
  using StorageType = T;
  static constexpr wdUInt32 GetNumBits() { return wdMath::NumBits<T>(); }

  /// \brief Initializes the bitfield to all zero.
  wdStaticBitfield();

  static wdStaticBitfield<T> FromMask(StorageType bits);

  /// \brief Returns true, if the bitfield is not zero.
  bool IsAnyBitSet() const; // [tested]

  /// \brief Returns true, if the bitfield is all zero.
  bool IsNoBitSet() const; // [tested]

  /// \brief Returns true, if the bitfield is not empty and all bits are set to one.
  bool AreAllBitsSet() const; // [tested]

  /// \brief Sets the given bit to 1.
  void SetBit(wdUInt32 uiBit); // [tested]

  /// \brief Clears the given bit to 0.
  void ClearBit(wdUInt32 uiBit); // [tested]

  /// \brief Returns true, if the given bit is set to 1.
  bool IsBitSet(wdUInt32 uiBit) const; // [tested]

  /// \brief Clears all bits to 0. Same as Clear().
  void ClearAllBits(); // [tested]

  /// \brief Sets all bits to 1.
  void SetAllBits(); // [tested]

  /// \brief Sets the range starting at uiFirstBit up to (and including) uiLastBit to 1.
  void SetBitRange(wdUInt32 uiFirstBit, wdUInt32 uiNumBits); // [tested]

  /// \brief Clears the range starting at uiFirstBit up to (and including) uiLastBit to 0.
  void ClearBitRange(wdUInt32 uiFirstBit, wdUInt32 uiNumBits); // [tested]

  /// \brief Returns the raw uint that stores all bits.
  T GetValue() const; // [tested]

  /// \brief Sets the raw uint that stores all bits.
  void SetValue(T value); // [tested]

  /// \brief Modifies \a this to also contain the bits from \a rhs.
  WD_ALWAYS_INLINE void operator|=(const wdStaticBitfield<T>& rhs) { m_Storage |= rhs.m_Storage; }

  /// \brief Modifies \a this to only contain the bits that were set in \a this and \a rhs.
  WD_ALWAYS_INLINE void operator&=(const wdStaticBitfield<T>& rhs) { m_Storage &= rhs.m_Storage; }

  wdResult Serialize(wdStreamWriter& inout_writer) const
  {
    inout_writer.WriteVersion(s_Version);
    inout_writer << m_Storage;
    return WD_SUCCESS;
  }

  wdResult Deserialize(wdStreamReader& inout_reader)
  {
    /*auto version =*/inout_reader.ReadVersion(s_Version);
    inout_reader >> m_Storage;
    return WD_SUCCESS;
  }

private:
  static constexpr wdTypeVersion s_Version = 1;

  wdStaticBitfield(StorageType initValue)
    : m_Storage(initValue)
  {
  }

  template <typename U>
  friend wdStaticBitfield<U> operator|(wdStaticBitfield<U> lhs, wdStaticBitfield<U> rhs);

  template <typename U>
  friend wdStaticBitfield<U> operator&(wdStaticBitfield<U> lhs, wdStaticBitfield<U> rhs);

  template <typename U>
  friend wdStaticBitfield<U> operator^(wdStaticBitfield<U> lhs, wdStaticBitfield<U> rhs);

  template <typename U>
  friend bool operator==(wdStaticBitfield<U> lhs, wdStaticBitfield<U> rhs);

  template <typename U>
  friend bool operator!=(wdStaticBitfield<U> lhs, wdStaticBitfield<U> rhs);

  StorageType m_Storage = 0;
};

template <typename T>
inline wdStaticBitfield<T> operator|(wdStaticBitfield<T> lhs, wdStaticBitfield<T> rhs)
{
  return wdStaticBitfield<T>(lhs.m_Storage | rhs.m_Storage);
}

template <typename T>
inline wdStaticBitfield<T> operator&(wdStaticBitfield<T> lhs, wdStaticBitfield<T> rhs)
{
  return wdStaticBitfield<T>(lhs.m_Storage & rhs.m_Storage);
}

template <typename T>
inline wdStaticBitfield<T> operator^(wdStaticBitfield<T> lhs, wdStaticBitfield<T> rhs)
{
  return wdStaticBitfield<T>(lhs.m_Storage ^ rhs.m_Storage);
}

template <typename T>
inline bool operator==(wdStaticBitfield<T> lhs, wdStaticBitfield<T> rhs)
{
  return lhs.m_Storage == rhs.m_Storage;
}

template <typename T>
inline bool operator!=(wdStaticBitfield<T> lhs, wdStaticBitfield<T> rhs)
{
  return lhs.m_Storage != rhs.m_Storage;
}

using wdStaticBitfield8 = wdStaticBitfield<wdUInt8>;
using wdStaticBitfield16 = wdStaticBitfield<wdUInt16>;
using wdStaticBitfield32 = wdStaticBitfield<wdUInt32>;
using wdStaticBitfield64 = wdStaticBitfield<wdUInt64>;

#include <Foundation/Containers/Implementation/Bitfield_inl.h>
