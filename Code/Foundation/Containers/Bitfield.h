#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Containers/Implementation/BitIterator.h>
#include <Foundation/IO/Stream.h>
#include <Foundation/Math/Constants.h>

/// \brief A template interface, that turns any array class into a bitfield.
///
/// This class provides an interface to work with single bits, to store true/false values.
/// The underlying container is configurable, though it must support random access and a 'SetCount' function and it must use elements of type
/// nsUInt32. In most cases a dynamic array should be used. For this case the nsDynamicBitfield typedef is already available. There is also an
/// nsHybridBitfield typedef.
template <class Container>
class nsBitfield
{
public:
  nsBitfield() = default;

  /// \brief Returns the number of bits that this bitfield stores.
  nsUInt32 GetCount() const; // [tested]

  /// \brief Resizes the Bitfield to hold the given number of bits. This version does NOT initialize new bits!
  template <typename = void>                       // Template is used to only conditionally compile this function in when it is actually used.
  void SetCountUninitialized(nsUInt32 uiBitCount); // [tested]

  /// \brief Resizes the Bitfield to hold the given number of bits. If \a bSetNew is true, new bits are set to 1, otherwise they are cleared to 0.
  void SetCount(nsUInt32 uiBitCount, bool bSetNew = false); // [tested]

  /// \brief Returns true, if the bitfield does not store any bits.
  bool IsEmpty() const; // [tested]

  /// \brief Returns true, if the bitfield is not empty and any bit is 1.
  bool IsAnyBitSet(nsUInt32 uiFirstBit = 0, nsUInt32 uiNumBits = 0xFFFFFFFF) const; // [tested]

  /// \brief Returns true, if the bitfield is empty or all bits are set to zero.
  bool IsNoBitSet(nsUInt32 uiFirstBit = 0, nsUInt32 uiNumBits = 0xFFFFFFFF) const; // [tested]

  /// \brief Returns true, if the bitfield is not empty and all bits are set to one.
  bool AreAllBitsSet(nsUInt32 uiFirstBit = 0, nsUInt32 uiNumBits = 0xFFFFFFFF) const; // [tested]

  /// \brief Discards all bits and sets count to zero.
  void Clear(); // [tested]

  /// \brief Sets the given bit to 1.
  void SetBit(nsUInt32 uiBit); // [tested]

  /// \brief Clears the given bit to 0.
  void ClearBit(nsUInt32 uiBit); // [tested]

  /// \brief Sets the given bit to 1 or 0 depending on the given value.
  void SetBitValue(nsUInt32 uiBit, bool bValue); // [tested]

  /// \brief Returns true, if the given bit is set to 1.
  bool IsBitSet(nsUInt32 uiBit) const; // [tested]

  /// \brief Clears all bits to 0.
  void ClearAllBits(); // [tested]

  /// \brief Sets all bits to 1.
  void SetAllBits(); // [tested]

  /// \brief Sets the range starting at uiFirstBit up to (and including) uiLastBit to 1.
  void SetBitRange(nsUInt32 uiFirstBit, nsUInt32 uiNumBits); // [tested]

  /// \brief Clears the range starting at uiFirstBit up to (and including) uiLastBit to 0.
  void ClearBitRange(nsUInt32 uiFirstBit, nsUInt32 uiNumBits); // [tested]

  /// \brief Swaps two bitfields
  void Swap(nsBitfield<Container>& other); // [tested]
  struct ConstIterator
  {
    using iterator_category = std::forward_iterator_tag;
    using value_type = nsUInt32;
    using sub_iterator = ::nsBitIterator<nsUInt32, true>;

    // Invalid iterator (end)
    NS_FORCE_INLINE ConstIterator() = default; // [tested]

    // Start iterator.
    explicit ConstIterator(const nsBitfield<Container>& bitfield); // [tested]

    /// \brief Checks whether this iterator points to a valid element.
    bool IsValid() const; // [tested]

    /// \brief Returns the 'value' of the element that this iterator points to.
    nsUInt32 Value() const; // [tested]

    /// \brief Advances the iterator to the next element in the map. The iterator will not be valid anymore, if the end is reached.
    void Next();                                       // [tested]

    bool operator==(const ConstIterator& other) const; // [tested]
    bool operator!=(const ConstIterator& other) const; // [tested]

    /// \brief Returns 'Value()' to enable foreach.
    nsUInt32 operator*() const; // [tested]

    /// \brief Shorthand for 'Next'.
    void operator++(); // [tested]

  private:
    void FindNextChunk(nsUInt32 uiStartChunk);

  private:
    nsUInt32 m_uiChunk = 0;
    sub_iterator m_Iterator;
    const nsBitfield<Container>* m_pBitfield = nullptr;
  };

  /// \brief Returns a constant iterator to the very first set bit.
  /// Note that due to the way iterating through bits is accelerated, changes to the bitfield while iterating through the bits has undefined behaviour.
  ConstIterator GetIterator() const; // [tested]

  /// \brief Returns an invalid iterator. Needed to support range based for loops.
  ConstIterator GetEndIterator() const; // [tested]

private:
  friend struct ConstIterator;

  nsUInt32 GetBitInt(nsUInt32 uiBitIndex) const;
  nsUInt32 GetBitMask(nsUInt32 uiBitIndex) const;

  nsUInt32 m_uiCount = 0;
  Container m_Container;
};

/// \brief This should be the main type of bitfield to use, although other internal container types are possible.
using nsDynamicBitfield = nsBitfield<nsDynamicArray<nsUInt32>>;

/// \brief An nsBitfield that uses a hybrid array as internal container.
template <nsUInt32 BITS>
using nsHybridBitfield = nsBitfield<nsHybridArray<nsUInt32, (BITS + 31) / 32>>;

//////////////////////////////////////////////////////////////////////////
// begin() /end() for range-based for-loop support
template <typename Container>
typename nsBitfield<Container>::ConstIterator begin(const nsBitfield<Container>& container)
{
  return container.GetIterator();
}

template <typename Container>
typename nsBitfield<Container>::ConstIterator cbegin(const nsBitfield<Container>& container)
{
  return container.GetIterator();
}

template <typename Container>
typename nsBitfield<Container>::ConstIterator end(const nsBitfield<Container>& container)
{
  return container.GetEndIterator();
}

template <typename Container>
typename nsBitfield<Container>::ConstIterator cend(const nsBitfield<Container>& container)
{
  return container.GetEndIterator();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

template <typename T>
class nsStaticBitfield
{
public:
  using StorageType = T;
  using ConstIterator = nsBitIterator<StorageType, true, nsUInt32>;

  static constexpr nsUInt32 GetStorageTypeBitCount() { return nsMath::NumBits<T>(); }

  /// \brief Initializes the bitfield to all zero.
  nsStaticBitfield();

  static nsStaticBitfield<T> MakeFromMask(StorageType bits);

  /// \brief Returns true, if the bitfield is not zero.
  bool IsAnyBitSet() const; // [tested]

  /// \brief Returns true, if the bitfield is all zero.
  bool IsNoBitSet() const; // [tested]

  /// \brief Returns true, if the bitfield is not empty and all bits are set to one.
  bool AreAllBitsSet() const; // [tested]

  /// \brief Sets the given bit to 1.
  void SetBit(nsUInt32 uiBit); // [tested]

  /// \brief Clears the given bit to 0.
  void ClearBit(nsUInt32 uiBit); // [tested]

  /// \brief Sets the given bit to 1 or 0 depending on the given value.
  void SetBitValue(nsUInt32 uiBit, bool bValue); // [tested]

  /// \brief Returns true, if the given bit is set to 1.
  bool IsBitSet(nsUInt32 uiBit) const; // [tested]

  /// \brief Clears all bits to 0. Same as Clear().
  void ClearAllBits(); // [tested]

  /// \brief Sets all bits to 1.
  void SetAllBits(); // [tested]

  /// \brief Sets the range starting at uiFirstBit up to (and including) uiLastBit to 1.
  void SetBitRange(nsUInt32 uiFirstBit, nsUInt32 uiNumBits); // [tested]

  /// \brief Clears the range starting at uiFirstBit up to (and including) uiLastBit to 0.
  void ClearBitRange(nsUInt32 uiFirstBit, nsUInt32 uiNumBits); // [tested]

  /// \brief Returns the index of the lowest bit that is set. Returns the max index+1 in case no bit is set, at all.
  nsUInt32 GetLowestBitSet() const; // [tested]

  /// \brief Returns the index of the highest bit that is set. Returns the max index+1 in case no bit is set, at all.
  nsUInt32 GetHighestBitSet() const; // [tested]

  /// \brief Returns the count of how many bits are set in total.
  nsUInt32 GetNumBitsSet() const; // [tested]

  /// \brief Returns the raw uint that stores all bits.
  T GetValue() const; // [tested]

  /// \brief Sets the raw uint that stores all bits.
  void SetValue(T value); // [tested]

  /// \brief Swaps two bitfields
  void Swap(nsStaticBitfield<T>& other); // [tested]

  /// \brief Modifies \a this to also contain the bits from \a rhs.
  NS_ALWAYS_INLINE void operator|=(const nsStaticBitfield<T>& rhs) { m_Storage |= rhs.m_Storage; }

  /// \brief Modifies \a this to only contain the bits that were set in \a this and \a rhs.
  NS_ALWAYS_INLINE void operator&=(const nsStaticBitfield<T>& rhs) { m_Storage &= rhs.m_Storage; }

  nsResult Serialize(nsStreamWriter& inout_writer) const
  {
    inout_writer.WriteVersion(s_Version);
    inout_writer << m_Storage;
    return NS_SUCCESS;
  }

  nsResult Deserialize(nsStreamReader& inout_reader)
  {
    /*auto version =*/inout_reader.ReadVersion(s_Version);
    inout_reader >> m_Storage;
    return NS_SUCCESS;
  }

  /// \brief Returns a constant iterator to the very first set bit.
  /// Note that due to the way iterating through bits is accelerated, changes to the bitfield while iterating through the bits has undefined behaviour.
  ConstIterator GetIterator() const // [tested]
  {
    return ConstIterator(m_Storage);
  };

  /// \brief Returns an invalid iterator. Needed to support range based for loops.
  ConstIterator GetEndIterator() const // [tested]
  {
    return ConstIterator();
  };

private:
  static constexpr nsTypeVersion s_Version = 1;

  nsStaticBitfield(StorageType initValue)
    : m_Storage(initValue)
  {
  }

  template <typename U>
  friend nsStaticBitfield<U> operator|(nsStaticBitfield<U> lhs, nsStaticBitfield<U> rhs);

  template <typename U>
  friend nsStaticBitfield<U> operator&(nsStaticBitfield<U> lhs, nsStaticBitfield<U> rhs);

  template <typename U>
  friend nsStaticBitfield<U> operator^(nsStaticBitfield<U> lhs, nsStaticBitfield<U> rhs);

  template <typename U>
  friend bool operator==(nsStaticBitfield<U> lhs, nsStaticBitfield<U> rhs);

  template <typename U>
  friend bool operator!=(nsStaticBitfield<U> lhs, nsStaticBitfield<U> rhs);

  StorageType m_Storage = 0;
};

template <typename T>
inline nsStaticBitfield<T> operator|(nsStaticBitfield<T> lhs, nsStaticBitfield<T> rhs)
{
  return nsStaticBitfield<T>(lhs.m_Storage | rhs.m_Storage);
}

template <typename T>
inline nsStaticBitfield<T> operator&(nsStaticBitfield<T> lhs, nsStaticBitfield<T> rhs)
{
  return nsStaticBitfield<T>(lhs.m_Storage & rhs.m_Storage);
}

template <typename T>
inline nsStaticBitfield<T> operator^(nsStaticBitfield<T> lhs, nsStaticBitfield<T> rhs)
{
  return nsStaticBitfield<T>(lhs.m_Storage ^ rhs.m_Storage);
}

template <typename T>
inline bool operator==(nsStaticBitfield<T> lhs, nsStaticBitfield<T> rhs)
{
  return lhs.m_Storage == rhs.m_Storage;
}

template <typename T>
inline bool operator!=(nsStaticBitfield<T> lhs, nsStaticBitfield<T> rhs)
{
  return lhs.m_Storage != rhs.m_Storage;
}

//////////////////////////////////////////////////////////////////////////
// begin() /end() for range-based for-loop support
template <typename Container>
typename nsStaticBitfield<Container>::ConstIterator begin(const nsStaticBitfield<Container>& container)
{
  return container.GetIterator();
}

template <typename Container>
typename nsStaticBitfield<Container>::ConstIterator cbegin(const nsStaticBitfield<Container>& container)
{
  return container.GetIterator();
}

template <typename Container>
typename nsStaticBitfield<Container>::ConstIterator end(const nsStaticBitfield<Container>& container)
{
  return container.GetEndIterator();
}

template <typename Container>
typename nsStaticBitfield<Container>::ConstIterator cend(const nsStaticBitfield<Container>& container)
{
  return container.GetEndIterator();
}

using nsStaticBitfield8 = nsStaticBitfield<nsUInt8>;
using nsStaticBitfield16 = nsStaticBitfield<nsUInt16>;
using nsStaticBitfield32 = nsStaticBitfield<nsUInt32>;
using nsStaticBitfield64 = nsStaticBitfield<nsUInt64>;

#include <Foundation/Containers/Implementation/Bitfield_inl.h>
