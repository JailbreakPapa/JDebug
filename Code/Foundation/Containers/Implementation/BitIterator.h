#pragma once

#include <Foundation/Math/Math.h>

/// Chooses either nsUInt32 or nsUInt64 as the storage type for a given type T depending on its size. Required as nsMath::FirstBitLow only supports nsUInt32 or nsUInt64.
/// \tparam T Type for which the storage should be inferred.
template <typename T, typename = std::void_t<>>
struct nsBitIteratorStorage;
template <typename T>
struct nsBitIteratorStorage<T, std::enable_if_t<sizeof(T) <= 4>>
{
  using Type = nsUInt32;
};
template <typename T>
struct nsBitIteratorStorage<T, std::enable_if_t<sizeof(T) >= 5>>
{
  using Type = nsUInt64;
};

/// Configurable bit iterator. Allows for iterating over the bits in an integer, returning either the bit index or value.
/// \tparam DataType The type of data that is being iterated over.
/// \tparam ReturnsIndex If set, returns the index of the bit. Otherwise returns the value of the bit, i.e. NS_BIT(value).
/// \tparam ReturnType Returned value type of the iterator. Defaults to same as DataType.
/// \tparam StorageType The storage type that the bit operations are performed on (either nsUInt32 or nsUInt64). Auto-computed.
template <typename DataType, bool ReturnsIndex = true, typename ReturnType = DataType, typename StorageType = typename nsBitIteratorStorage<DataType>::Type>
struct nsBitIterator
{
  using iterator_category = std::forward_iterator_tag;
  using value_type = DataType;
  NS_CHECK_AT_COMPILETIME(sizeof(DataType) <= 8);

  // Invalid iterator (end)
  NS_ALWAYS_INLINE nsBitIterator() = default;

  // Start iterator.
  NS_ALWAYS_INLINE explicit nsBitIterator(DataType data)
  {
    m_uiMask = static_cast<StorageType>(data);
  }

  NS_ALWAYS_INLINE bool IsValid() const
  {
    return m_uiMask != 0;
  }

  NS_ALWAYS_INLINE ReturnType Value() const
  {
    if constexpr (ReturnsIndex)
    {
      return static_cast<ReturnType>(nsMath::FirstBitLow(m_uiMask));
    }
    else
    {
      return static_cast<ReturnType>(NS_BIT(nsMath::FirstBitLow(m_uiMask)));
    }
  }

  NS_ALWAYS_INLINE void Next()
  {
    // Clear the lowest set bit. Why this works: https://www.geeksforgeeks.org/turn-off-the-rightmost-set-bit/
    m_uiMask = m_uiMask & (m_uiMask - 1);
  }

  NS_ALWAYS_INLINE bool operator==(const nsBitIterator& other) const
  {
    return m_uiMask == other.m_uiMask;
  }

  NS_ALWAYS_INLINE bool operator!=(const nsBitIterator& other) const
  {
    return m_uiMask != other.m_uiMask;
  }

  NS_ALWAYS_INLINE ReturnType operator*() const
  {
    return Value();
  }

  NS_ALWAYS_INLINE void operator++()
  {
    Next();
  }

  StorageType m_uiMask = 0;
};
