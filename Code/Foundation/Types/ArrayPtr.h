#pragma once

#include <Foundation/Memory/MemoryUtils.h>

#include <Foundation/Containers/Implementation/ArrayIterator.h>

// This #include is quite vital, do not remove it!
#include <Foundation/Strings/FormatString.h>

#include <Foundation/Math/Math.h>

/// \brief Value used by containers for indices to indicate an invalid index.
#ifndef wdInvalidIndex
#  define wdInvalidIndex 0xFFFFFFFF
#endif

namespace wdArrayPtrDetail
{
  template <typename U>
  struct ByteTypeHelper
  {
    using type = wdUInt8;
  };

  template <typename U>
  struct ByteTypeHelper<const U>
  {
    using type = const wdUInt8;
  };
} // namespace wdArrayPtrDetail

/// \brief This class encapsulates an array and it's size. It is recommended to use this class instead of plain C arrays.
///
/// No data is deallocated at destruction, the wdArrayPtr only allows for easier access.
template <typename T>
class wdArrayPtr
{
  template <typename U>
  friend class wdArrayPtr;

public:
  WD_DECLARE_POD_TYPE();

  static_assert(!std::is_same_v<T, void>, "wdArrayPtr<void> is not allowed (anymore)");
  static_assert(!std::is_same_v<T, const void>, "wdArrayPtr<void> is not allowed (anymore)");

  using ByteType = typename wdArrayPtrDetail::ByteTypeHelper<T>::type;
  using ValueType = T;
  using PointerType = T*;

  /// \brief Initializes the wdArrayPtr to be empty.
  WD_ALWAYS_INLINE wdArrayPtr() // [tested]
    : m_pPtr(nullptr)
    , m_uiCount(0u)
  {
  }

  /// \brief Copies the pointer and size of /a other. Does not allocate any data.
  WD_ALWAYS_INLINE wdArrayPtr(const wdArrayPtr<T>& other) // [tested]
  {
    m_pPtr = other.m_pPtr;
    m_uiCount = other.m_uiCount;
  }

  /// \brief Initializes the wdArrayPtr with the given pointer and number of elements. No memory is allocated or copied.
  inline wdArrayPtr(T* pPtr, wdUInt32 uiCount) // [tested]
    : m_pPtr(pPtr)
    , m_uiCount(uiCount)
  {
    // If any of the arguments is invalid, we invalidate ourself.
    if (m_pPtr == nullptr || m_uiCount == 0)
    {
      m_pPtr = nullptr;
      m_uiCount = 0;
    }
  }

  /// \brief Initializes the wdArrayPtr to encapsulate the given array.
  template <size_t N>
  WD_ALWAYS_INLINE wdArrayPtr(T (&staticArray)[N]) // [tested]
    : m_pPtr(staticArray)
    , m_uiCount(static_cast<wdUInt32>(N))
  {
  }

  /// \brief Initializes the wdArrayPtr to be a copy of \a other. No memory is allocated or copied.
  template <typename U>
  WD_ALWAYS_INLINE wdArrayPtr(const wdArrayPtr<U>& other) // [tested]
    : m_pPtr(other.m_pPtr)
    , m_uiCount(other.m_uiCount)
  {
  }

  /// \brief Convert to const version.
  operator wdArrayPtr<const T>() const { return wdArrayPtr<const T>(static_cast<const T*>(GetPtr()), GetCount()); } // [tested]

  /// \brief Copies the pointer and size of /a other. Does not allocate any data.
  WD_ALWAYS_INLINE void operator=(const wdArrayPtr<T>& other) // [tested]
  {
    m_pPtr = other.m_pPtr;
    m_uiCount = other.m_uiCount;
  }

  /// \brief Clears the array
  WD_ALWAYS_INLINE void Clear()
  {
    m_pPtr = nullptr;
    m_uiCount = 0;
  }

  WD_ALWAYS_INLINE void operator=(std::nullptr_t) // [tested]
  {
    m_pPtr = nullptr;
    m_uiCount = 0;
  }

  /// \brief Returns the pointer to the array.
  WD_ALWAYS_INLINE PointerType GetPtr() const // [tested]
  {
    return m_pPtr;
  }

  /// \brief Returns the pointer to the array.
  WD_ALWAYS_INLINE PointerType GetPtr() // [tested]
  {
    return m_pPtr;
  }

  /// \brief Returns the pointer behind the last element of the array
  WD_ALWAYS_INLINE PointerType GetEndPtr() { return m_pPtr + m_uiCount; }

  /// \brief Returns the pointer behind the last element of the array
  WD_ALWAYS_INLINE PointerType GetEndPtr() const { return m_pPtr + m_uiCount; }

  /// \brief Returns whether the array is empty.
  WD_ALWAYS_INLINE bool IsEmpty() const // [tested]
  {
    return GetCount() == 0;
  }

  /// \brief Returns the number of elements in the array.
  WD_ALWAYS_INLINE wdUInt32 GetCount() const // [tested]
  {
    return m_uiCount;
  }

  /// \brief Creates a sub-array from this array.
  WD_FORCE_INLINE wdArrayPtr<T> GetSubArray(wdUInt32 uiStart, wdUInt32 uiCount) const // [tested]
  {
    // the first check is necessary to also detect errors when uiStart+uiCount would overflow
    WD_ASSERT_DEV(uiStart <= GetCount() && uiStart + uiCount <= GetCount(), "uiStart+uiCount ({0}) has to be smaller or equal than the count ({1}).",
      uiStart + uiCount, GetCount());
    return wdArrayPtr<T>(GetPtr() + uiStart, uiCount);
  }

  /// \brief Creates a sub-array from this array.
  /// \note \code ap.GetSubArray(i) \endcode is equivalent to \code ap.GetSubArray(i, ap.GetCount() - i) \endcode.
  WD_FORCE_INLINE wdArrayPtr<T> GetSubArray(wdUInt32 uiStart) const // [tested]
  {
    WD_ASSERT_DEV(uiStart <= GetCount(), "uiStart ({0}) has to be smaller or equal than the count ({1}).", uiStart, GetCount());
    return wdArrayPtr<T>(GetPtr() + uiStart, GetCount() - uiStart);
  }

  /// \brief Reinterprets this array as a byte array.
  WD_ALWAYS_INLINE wdArrayPtr<const ByteType> ToByteArray() const
  {
    return wdArrayPtr<const ByteType>(reinterpret_cast<const ByteType*>(GetPtr()), GetCount() * sizeof(T));
  }

  /// \brief Reinterprets this array as a byte array.
  WD_ALWAYS_INLINE wdArrayPtr<ByteType> ToByteArray() { return wdArrayPtr<ByteType>(reinterpret_cast<ByteType*>(GetPtr()), GetCount() * sizeof(T)); }


  /// \brief Cast an ArrayPtr to an ArrayPtr to a different, but same size, type
  template <typename U>
  WD_ALWAYS_INLINE wdArrayPtr<U> Cast()
  {
    static_assert(sizeof(T) == sizeof(U), "Can only cast with equivalent element size.");
    return wdArrayPtr<U>(reinterpret_cast<U*>(GetPtr()), GetCount());
  }

  /// \brief Cast an ArrayPtr to an ArrayPtr to a different, but same size, type
  template <typename U>
  WD_ALWAYS_INLINE wdArrayPtr<const U> Cast() const
  {
    static_assert(sizeof(T) == sizeof(U), "Can only cast with equivalent element size.");
    return wdArrayPtr<const U>(reinterpret_cast<const U*>(GetPtr()), GetCount());
  }

  /// \brief Index access.
  WD_FORCE_INLINE const ValueType& operator[](wdUInt32 uiIndex) const // [tested]
  {
    WD_ASSERT_DEV(uiIndex < GetCount(), "Cannot access element {0}, the array only holds {1} elements.", uiIndex, GetCount());
    return *static_cast<const ValueType*>(GetPtr() + uiIndex);
  }

  /// \brief Index access.
  WD_FORCE_INLINE ValueType& operator[](wdUInt32 uiIndex) // [tested]
  {
    WD_ASSERT_DEV(uiIndex < GetCount(), "Cannot access element {0}, the array only holds {1} elements.", uiIndex, GetCount());
    return *static_cast<ValueType*>(GetPtr() + uiIndex);
  }

  /// \brief Compares the two arrays for equality.
  inline bool operator==(const wdArrayPtr<const T>& other) const // [tested]
  {
    if (GetCount() != other.GetCount())
      return false;

    if (GetPtr() == other.GetPtr())
      return true;

    return wdMemoryUtils::IsEqual(static_cast<const ValueType*>(GetPtr()), static_cast<const ValueType*>(other.GetPtr()), GetCount());
  }

  /// \brief Compares the two arrays for inequality.
  WD_ALWAYS_INLINE bool operator!=(const wdArrayPtr<const T>& other) const // [tested]
  {
    return !(*this == other);
  }

  /// \brief Compares the two arrays for less.
  inline bool operator<(const wdArrayPtr<const T>& other) const // [tested]
  {
    if (GetCount() != other.GetCount())
      return GetCount() < other.GetCount();

    for (wdUInt32 i = 0; i < GetCount(); ++i)
    {
      if (GetPtr()[i] < other.GetPtr()[i])
        return true;

      if (other.GetPtr()[i] < GetPtr()[i])
        return false;
    }

    return false;
  }

  /// \brief Copies the data from \a other into this array. The arrays must have the exact same size.
  inline void CopyFrom(const wdArrayPtr<const T>& other) // [tested]
  {
    WD_ASSERT_DEV(
      GetCount() == other.GetCount(), "Count for copy does not match. Target has {0} elements, source {1} elements", GetCount(), other.GetCount());

    wdMemoryUtils::Copy(static_cast<ValueType*>(GetPtr()), static_cast<const ValueType*>(other.GetPtr()), GetCount());
  }

  WD_ALWAYS_INLINE void Swap(wdArrayPtr<T>& other)
  {
    ::wdMath::Swap(m_pPtr, other.m_pPtr);
    ::wdMath::Swap(m_uiCount, other.m_uiCount);
  }

  /// \brief Checks whether the given value can be found in the array. O(n) complexity.
  WD_ALWAYS_INLINE bool Contains(const T& value) const // [tested]
  {
    return IndexOf(value) != wdInvalidIndex;
  }

  /// \brief Searches for the first occurrence of the given value and returns its index or wdInvalidIndex if not found.
  inline wdUInt32 IndexOf(const T& value, wdUInt32 uiStartIndex = 0) const // [tested]
  {
    for (wdUInt32 i = uiStartIndex; i < m_uiCount; ++i)
    {
      if (wdMemoryUtils::IsEqual(m_pPtr + i, &value))
        return i;
    }

    return wdInvalidIndex;
  }

  /// \brief Searches for the last occurrence of the given value and returns its index or wdInvalidIndex if not found.
  inline wdUInt32 LastIndexOf(const T& value, wdUInt32 uiStartIndex = wdInvalidIndex) const // [tested]
  {
    for (wdUInt32 i = ::wdMath::Min(uiStartIndex, m_uiCount); i-- > 0;)
    {
      if (wdMemoryUtils::IsEqual(m_pPtr + i, &value))
        return i;
    }
    return wdInvalidIndex;
  }

  using const_iterator = const T*;
  using const_reverse_iterator = const_reverse_pointer_iterator<T>;
  using iterator = T*;
  using reverse_iterator = reverse_pointer_iterator<T>;

private:
  PointerType m_pPtr;
  wdUInt32 m_uiCount;
};

//////////////////////////////////////////////////////////////////////////

using wdByteArrayPtr = wdArrayPtr<wdUInt8>;
using wdConstByteArrayPtr = wdArrayPtr<const wdUInt8>;

//////////////////////////////////////////////////////////////////////////

/// \brief Helper function to create wdArrayPtr from a pointer of some type and a count.
template <typename T>
WD_ALWAYS_INLINE wdArrayPtr<T> wdMakeArrayPtr(T* pPtr, wdUInt32 uiCount)
{
  return wdArrayPtr<T>(pPtr, uiCount);
}

/// \brief Helper function to create wdArrayPtr from a static array the a size known at compile-time.
template <typename T, wdUInt32 N>
WD_ALWAYS_INLINE wdArrayPtr<T> wdMakeArrayPtr(T (&staticArray)[N])
{
  return wdArrayPtr<T>(staticArray);
}

/// \brief Helper function to create wdConstByteArrayPtr from a pointer of some type and a count.
template <typename T>
WD_ALWAYS_INLINE wdConstByteArrayPtr wdMakeByteArrayPtr(const T* pPtr, wdUInt32 uiCount)
{
  return wdConstByteArrayPtr(static_cast<const wdUInt8*>(pPtr), uiCount * sizeof(T));
}

/// \brief Helper function to create wdByteArrayPtr from a pointer of some type and a count.
template <typename T>
WD_ALWAYS_INLINE wdByteArrayPtr wdMakeByteArrayPtr(T* pPtr, wdUInt32 uiCount)
{
  return wdByteArrayPtr(reinterpret_cast<wdUInt8*>(pPtr), uiCount * sizeof(T));
}

/// \brief Helper function to create wdByteArrayPtr from a void pointer and a count.
WD_ALWAYS_INLINE wdByteArrayPtr wdMakeByteArrayPtr(void* pPtr, wdUInt32 uiBytes)
{
  return wdByteArrayPtr(reinterpret_cast<wdUInt8*>(pPtr), uiBytes);
}

/// \brief Helper function to create wdConstByteArrayPtr from a const void pointer and a count.
WD_ALWAYS_INLINE wdConstByteArrayPtr wdMakeByteArrayPtr(const void* pPtr, wdUInt32 uiBytes)
{
  return wdConstByteArrayPtr(static_cast<const wdUInt8*>(pPtr), uiBytes);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
typename wdArrayPtr<T>::iterator begin(wdArrayPtr<T>& ref_container)
{
  return ref_container.GetPtr();
}

template <typename T>
typename wdArrayPtr<T>::const_iterator begin(const wdArrayPtr<T>& container)
{
  return container.GetPtr();
}

template <typename T>
typename wdArrayPtr<T>::const_iterator cbegin(const wdArrayPtr<T>& container)
{
  return container.GetPtr();
}

template <typename T>
typename wdArrayPtr<T>::reverse_iterator rbegin(wdArrayPtr<T>& ref_container)
{
  return typename wdArrayPtr<T>::reverse_iterator(ref_container.GetPtr() + ref_container.GetCount() - 1);
}

template <typename T>
typename wdArrayPtr<T>::const_reverse_iterator rbegin(const wdArrayPtr<T>& container)
{
  return typename wdArrayPtr<T>::const_reverse_iterator(container.GetPtr() + container.GetCount() - 1);
}

template <typename T>
typename wdArrayPtr<T>::const_reverse_iterator crbegin(const wdArrayPtr<T>& container)
{
  return typename wdArrayPtr<T>::const_reverse_iterator(container.GetPtr() + container.GetCount() - 1);
}

template <typename T>
typename wdArrayPtr<T>::iterator end(wdArrayPtr<T>& ref_container)
{
  return ref_container.GetPtr() + ref_container.GetCount();
}

template <typename T>
typename wdArrayPtr<T>::const_iterator end(const wdArrayPtr<T>& container)
{
  return container.GetPtr() + container.GetCount();
}

template <typename T>
typename wdArrayPtr<T>::const_iterator cend(const wdArrayPtr<T>& container)
{
  return container.GetPtr() + container.GetCount();
}

template <typename T>
typename wdArrayPtr<T>::reverse_iterator rend(wdArrayPtr<T>& ref_container)
{
  return typename wdArrayPtr<T>::reverse_iterator(ref_container.GetPtr() - 1);
}

template <typename T>
typename wdArrayPtr<T>::const_reverse_iterator rend(const wdArrayPtr<T>& container)
{
  return typename wdArrayPtr<T>::const_reverse_iterator(container.GetPtr() - 1);
}

template <typename T>
typename wdArrayPtr<T>::const_reverse_iterator crend(const wdArrayPtr<T>& container)
{
  return typename wdArrayPtr<T>::const_reverse_iterator(container.GetPtr() - 1);
}
