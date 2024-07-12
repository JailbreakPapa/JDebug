#pragma once

#include <Foundation/Memory/MemoryUtils.h>

#include <Foundation/Containers/Implementation/ArrayIterator.h>

// This #include is quite vital, do not remove it!
#include <Foundation/Strings/FormatString.h>

#include <Foundation/Math/Math.h>

/// \brief Value used by containers for indices to indicate an invalid index.
#ifndef nsInvalidIndex
#  define nsInvalidIndex 0xFFFFFFFF
#endif

namespace nsArrayPtrDetail
{
  template <typename U>
  struct ByteTypeHelper
  {
    using type = nsUInt8;
  };

  template <typename U>
  struct ByteTypeHelper<const U>
  {
    using type = const nsUInt8;
  };
} // namespace nsArrayPtrDetail

/// \brief This class encapsulates an array and it's size. It is recommended to use this class instead of plain C arrays.
///
/// No data is deallocated at destruction, the nsArrayPtr only allows for easier access.
template <typename T>
class nsArrayPtr
{
  template <typename U>
  friend class nsArrayPtr;

public:
  NS_DECLARE_POD_TYPE();

  static_assert(!std::is_same_v<T, void>, "nsArrayPtr<void> is not allowed (anymore)");
  static_assert(!std::is_same_v<T, const void>, "nsArrayPtr<void> is not allowed (anymore)");

  using ByteType = typename nsArrayPtrDetail::ByteTypeHelper<T>::type;
  using ValueType = T;
  using PointerType = T*;

  /// \brief Initializes the nsArrayPtr to be empty.
  NS_ALWAYS_INLINE nsArrayPtr() // [tested]
    : m_pPtr(nullptr)
    , m_uiCount(0u)
  {
  }

  /// \brief Copies the pointer and size of /a other. Does not allocate any data.
  NS_ALWAYS_INLINE nsArrayPtr(const nsArrayPtr<T>& other) // [tested]
  {
    m_pPtr = other.m_pPtr;
    m_uiCount = other.m_uiCount;
  }

  /// \brief Initializes the nsArrayPtr with the given pointer and number of elements. No memory is allocated or copied.
  inline nsArrayPtr(T* pPtr, nsUInt32 uiCount) // [tested]
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

  /// \brief Initializes the nsArrayPtr to encapsulate the given array.
  template <size_t N>
  NS_ALWAYS_INLINE nsArrayPtr(T (&staticArray)[N]) // [tested]
    : m_pPtr(staticArray)
    , m_uiCount(static_cast<nsUInt32>(N))
  {
  }

  /// \brief Initializes the nsArrayPtr to be a copy of \a other. No memory is allocated or copied.
  template <typename U>
  NS_ALWAYS_INLINE nsArrayPtr(const nsArrayPtr<U>& other) // [tested]
    : m_pPtr(other.m_pPtr)
    , m_uiCount(other.m_uiCount)
  {
  }

  /// \brief Convert to const version.
  operator nsArrayPtr<const T>() const { return nsArrayPtr<const T>(static_cast<const T*>(GetPtr()), GetCount()); } // [tested]

  /// \brief Copies the pointer and size of /a other. Does not allocate any data.
  NS_ALWAYS_INLINE void operator=(const nsArrayPtr<T>& other) // [tested]
  {
    m_pPtr = other.m_pPtr;
    m_uiCount = other.m_uiCount;
  }

  /// \brief Clears the array
  NS_ALWAYS_INLINE void Clear()
  {
    m_pPtr = nullptr;
    m_uiCount = 0;
  }

  NS_ALWAYS_INLINE void operator=(std::nullptr_t) // [tested]
  {
    m_pPtr = nullptr;
    m_uiCount = 0;
  }

  /// \brief Returns the pointer to the array.
  NS_ALWAYS_INLINE PointerType GetPtr() const // [tested]
  {
    return m_pPtr;
  }

  /// \brief Returns the pointer to the array.
  NS_ALWAYS_INLINE PointerType GetPtr() // [tested]
  {
    return m_pPtr;
  }

  /// \brief Returns the pointer behind the last element of the array
  NS_ALWAYS_INLINE PointerType GetEndPtr() { return m_pPtr + m_uiCount; }

  /// \brief Returns the pointer behind the last element of the array
  NS_ALWAYS_INLINE PointerType GetEndPtr() const { return m_pPtr + m_uiCount; }

  /// \brief Returns whether the array is empty.
  NS_ALWAYS_INLINE bool IsEmpty() const // [tested]
  {
    return GetCount() == 0;
  }

  /// \brief Returns the number of elements in the array.
  NS_ALWAYS_INLINE nsUInt32 GetCount() const // [tested]
  {
    return m_uiCount;
  }

  /// \brief Creates a sub-array from this array.
  NS_FORCE_INLINE nsArrayPtr<T> GetSubArray(nsUInt32 uiStart, nsUInt32 uiCount) const // [tested]
  {
    // the first check is necessary to also detect errors when uiStart+uiCount would overflow
    NS_ASSERT_DEV(uiStart <= GetCount() && uiStart + uiCount <= GetCount(), "uiStart+uiCount ({0}) has to be smaller or equal than the count ({1}).",
      uiStart + uiCount, GetCount());
    return nsArrayPtr<T>(GetPtr() + uiStart, uiCount);
  }

  /// \brief Creates a sub-array from this array.
  /// \note \code ap.GetSubArray(i) \endcode is equivalent to \code ap.GetSubArray(i, ap.GetCount() - i) \endcode.
  NS_FORCE_INLINE nsArrayPtr<T> GetSubArray(nsUInt32 uiStart) const // [tested]
  {
    NS_ASSERT_DEV(uiStart <= GetCount(), "uiStart ({0}) has to be smaller or equal than the count ({1}).", uiStart, GetCount());
    return nsArrayPtr<T>(GetPtr() + uiStart, GetCount() - uiStart);
  }

  /// \brief Reinterprets this array as a byte array.
  NS_ALWAYS_INLINE nsArrayPtr<const ByteType> ToByteArray() const
  {
    return nsArrayPtr<const ByteType>(reinterpret_cast<const ByteType*>(GetPtr()), GetCount() * sizeof(T));
  }

  /// \brief Reinterprets this array as a byte array.
  NS_ALWAYS_INLINE nsArrayPtr<ByteType> ToByteArray() { return nsArrayPtr<ByteType>(reinterpret_cast<ByteType*>(GetPtr()), GetCount() * sizeof(T)); }


  /// \brief Cast an ArrayPtr to an ArrayPtr to a different, but same size, type
  template <typename U>
  NS_ALWAYS_INLINE nsArrayPtr<U> Cast()
  {
    static_assert(sizeof(T) == sizeof(U), "Can only cast with equivalent element size.");
    return nsArrayPtr<U>(reinterpret_cast<U*>(GetPtr()), GetCount());
  }

  /// \brief Cast an ArrayPtr to an ArrayPtr to a different, but same size, type
  template <typename U>
  NS_ALWAYS_INLINE nsArrayPtr<const U> Cast() const
  {
    static_assert(sizeof(T) == sizeof(U), "Can only cast with equivalent element size.");
    return nsArrayPtr<const U>(reinterpret_cast<const U*>(GetPtr()), GetCount());
  }

  /// \brief Index access.
  NS_FORCE_INLINE const ValueType& operator[](nsUInt32 uiIndex) const // [tested]
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(), "Cannot access element {0}, the array only holds {1} elements.", uiIndex, GetCount());
    return *static_cast<const ValueType*>(GetPtr() + uiIndex);
  }

  /// \brief Index access.
  NS_FORCE_INLINE ValueType& operator[](nsUInt32 uiIndex) // [tested]
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(), "Cannot access element {0}, the array only holds {1} elements.", uiIndex, GetCount());
    return *static_cast<ValueType*>(GetPtr() + uiIndex);
  }

  /// \brief Compares the two arrays for equality.
  template <typename = typename std::enable_if<std::is_const<T>::value == false>>
  inline bool operator==(const nsArrayPtr<const T>& other) const // [tested]
  {
    if (GetCount() != other.GetCount())
      return false;

    if (GetPtr() == other.GetPtr())
      return true;

    return nsMemoryUtils::IsEqual(static_cast<const ValueType*>(GetPtr()), static_cast<const ValueType*>(other.GetPtr()), GetCount());
  }

#if NS_DISABLED(NS_USE_CPP20_OPERATORS)
  template <typename = typename std::enable_if<std::is_const<T>::value == false>>
  inline bool operator!=(const nsArrayPtr<const T>& other) const // [tested]
  {
    return !(*this == other);
  }
#endif

  /// \brief Compares the two arrays for equality.
  inline bool operator==(const nsArrayPtr<T>& other) const // [tested]
  {
    if (GetCount() != other.GetCount())
      return false;

    if (GetPtr() == other.GetPtr())
      return true;

    return nsMemoryUtils::IsEqual(static_cast<const ValueType*>(GetPtr()), static_cast<const ValueType*>(other.GetPtr()), GetCount());
  }
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsArrayPtr<T>&);

  /// \brief Compares the two arrays for less.
  inline bool operator<(const nsArrayPtr<const T>& other) const // [tested]
  {
    if (GetCount() != other.GetCount())
      return GetCount() < other.GetCount();

    for (nsUInt32 i = 0; i < GetCount(); ++i)
    {
      if (GetPtr()[i] < other.GetPtr()[i])
        return true;

      if (other.GetPtr()[i] < GetPtr()[i])
        return false;
    }

    return false;
  }

  /// \brief Copies the data from \a other into this array. The arrays must have the exact same size.
  inline void CopyFrom(const nsArrayPtr<const T>& other) // [tested]
  {
    NS_ASSERT_DEV(GetCount() == other.GetCount(), "Count for copy does not match. Target has {0} elements, source {1} elements", GetCount(), other.GetCount());

    nsMemoryUtils::Copy(static_cast<ValueType*>(GetPtr()), static_cast<const ValueType*>(other.GetPtr()), GetCount());
  }

  NS_ALWAYS_INLINE void Swap(nsArrayPtr<T>& other)
  {
    ::nsMath::Swap(m_pPtr, other.m_pPtr);
    ::nsMath::Swap(m_uiCount, other.m_uiCount);
  }

  /// \brief Checks whether the given value can be found in the array. O(n) complexity.
  NS_ALWAYS_INLINE bool Contains(const T& value) const // [tested]
  {
    return IndexOf(value) != nsInvalidIndex;
  }

  /// \brief Searches for the first occurrence of the given value and returns its index or nsInvalidIndex if not found.
  inline nsUInt32 IndexOf(const T& value, nsUInt32 uiStartIndex = 0) const // [tested]
  {
    for (nsUInt32 i = uiStartIndex; i < m_uiCount; ++i)
    {
      if (nsMemoryUtils::IsEqual(m_pPtr + i, &value))
        return i;
    }

    return nsInvalidIndex;
  }

  /// \brief Searches for the last occurrence of the given value and returns its index or nsInvalidIndex if not found.
  inline nsUInt32 LastIndexOf(const T& value, nsUInt32 uiStartIndex = nsInvalidIndex) const // [tested]
  {
    for (nsUInt32 i = ::nsMath::Min(uiStartIndex, m_uiCount); i-- > 0;)
    {
      if (nsMemoryUtils::IsEqual(m_pPtr + i, &value))
        return i;
    }
    return nsInvalidIndex;
  }

  using const_iterator = const T*;
  using const_reverse_iterator = const_reverse_pointer_iterator<T>;
  using iterator = T*;
  using reverse_iterator = reverse_pointer_iterator<T>;

private:
  PointerType m_pPtr;
  nsUInt32 m_uiCount;
};

//////////////////////////////////////////////////////////////////////////

using nsByteArrayPtr = nsArrayPtr<nsUInt8>;
using nsConstByteArrayPtr = nsArrayPtr<const nsUInt8>;

//////////////////////////////////////////////////////////////////////////

/// \brief Helper function to create nsArrayPtr from a pointer of some type and a count.
template <typename T>
NS_ALWAYS_INLINE nsArrayPtr<T> nsMakeArrayPtr(T* pPtr, nsUInt32 uiCount)
{
  return nsArrayPtr<T>(pPtr, uiCount);
}

/// \brief Helper function to create nsArrayPtr from a static array the a size known at compile-time.
template <typename T, nsUInt32 N>
NS_ALWAYS_INLINE nsArrayPtr<T> nsMakeArrayPtr(T (&staticArray)[N])
{
  return nsArrayPtr<T>(staticArray);
}

/// \brief Helper function to create nsConstByteArrayPtr from a pointer of some type and a count.
template <typename T>
NS_ALWAYS_INLINE nsConstByteArrayPtr nsMakeByteArrayPtr(const T* pPtr, nsUInt32 uiCount)
{
  return nsConstByteArrayPtr(static_cast<const nsUInt8*>(pPtr), uiCount * sizeof(T));
}

/// \brief Helper function to create nsByteArrayPtr from a pointer of some type and a count.
template <typename T>
NS_ALWAYS_INLINE nsByteArrayPtr nsMakeByteArrayPtr(T* pPtr, nsUInt32 uiCount)
{
  return nsByteArrayPtr(reinterpret_cast<nsUInt8*>(pPtr), uiCount * sizeof(T));
}

/// \brief Helper function to create nsByteArrayPtr from a void pointer and a count.
NS_ALWAYS_INLINE nsByteArrayPtr nsMakeByteArrayPtr(void* pPtr, nsUInt32 uiBytes)
{
  return nsByteArrayPtr(reinterpret_cast<nsUInt8*>(pPtr), uiBytes);
}

/// \brief Helper function to create nsConstByteArrayPtr from a const void pointer and a count.
NS_ALWAYS_INLINE nsConstByteArrayPtr nsMakeByteArrayPtr(const void* pPtr, nsUInt32 uiBytes)
{
  return nsConstByteArrayPtr(static_cast<const nsUInt8*>(pPtr), uiBytes);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
typename nsArrayPtr<T>::iterator begin(nsArrayPtr<T>& ref_container)
{
  return ref_container.GetPtr();
}

template <typename T>
typename nsArrayPtr<T>::const_iterator begin(const nsArrayPtr<T>& container)
{
  return container.GetPtr();
}

template <typename T>
typename nsArrayPtr<T>::const_iterator cbegin(const nsArrayPtr<T>& container)
{
  return container.GetPtr();
}

template <typename T>
typename nsArrayPtr<T>::reverse_iterator rbegin(nsArrayPtr<T>& ref_container)
{
  return typename nsArrayPtr<T>::reverse_iterator(ref_container.GetPtr() + ref_container.GetCount() - 1);
}

template <typename T>
typename nsArrayPtr<T>::const_reverse_iterator rbegin(const nsArrayPtr<T>& container)
{
  return typename nsArrayPtr<T>::const_reverse_iterator(container.GetPtr() + container.GetCount() - 1);
}

template <typename T>
typename nsArrayPtr<T>::const_reverse_iterator crbegin(const nsArrayPtr<T>& container)
{
  return typename nsArrayPtr<T>::const_reverse_iterator(container.GetPtr() + container.GetCount() - 1);
}

template <typename T>
typename nsArrayPtr<T>::iterator end(nsArrayPtr<T>& ref_container)
{
  return ref_container.GetPtr() + ref_container.GetCount();
}

template <typename T>
typename nsArrayPtr<T>::const_iterator end(const nsArrayPtr<T>& container)
{
  return container.GetPtr() + container.GetCount();
}

template <typename T>
typename nsArrayPtr<T>::const_iterator cend(const nsArrayPtr<T>& container)
{
  return container.GetPtr() + container.GetCount();
}

template <typename T>
typename nsArrayPtr<T>::reverse_iterator rend(nsArrayPtr<T>& ref_container)
{
  return typename nsArrayPtr<T>::reverse_iterator(ref_container.GetPtr() - 1);
}

template <typename T>
typename nsArrayPtr<T>::const_reverse_iterator rend(const nsArrayPtr<T>& container)
{
  return typename nsArrayPtr<T>::const_reverse_iterator(container.GetPtr() - 1);
}

template <typename T>
typename nsArrayPtr<T>::const_reverse_iterator crend(const nsArrayPtr<T>& container)
{
  return typename nsArrayPtr<T>::const_reverse_iterator(container.GetPtr() - 1);
}
