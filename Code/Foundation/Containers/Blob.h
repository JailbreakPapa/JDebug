
#pragma once

#include <Foundation/Basics.h>

/// \brief This class encapsulates a blob's storage and it's size. It is recommended to use this class instead of directly working on the void* of the
/// blob.
///
/// No data is deallocated at destruction, the wdBlobPtr only allows for easier access.
template <typename T>
class wdBlobPtr
{
public:
  WD_DECLARE_POD_TYPE();

  static_assert(!std::is_same_v<T, void>, "wdBlobPtr<void> is not allowed (anymore)");
  static_assert(!std::is_same_v<T, const void>, "wdBlobPtr<void> is not allowed (anymore)");

  using ByteType = typename wdArrayPtrDetail::ByteTypeHelper<T>::type;
  using ValueType = T;
  using PointerType = T*;

  /// \brief Initializes the wdBlobPtr to be empty.
  WD_ALWAYS_INLINE wdBlobPtr()
    : m_pPtr(nullptr)
    , m_uiCount(0u)
  {
  }

  /// \brief Initializes the wdBlobPtr with the given pointer and number of elements. No memory is allocated or copied.
  template <typename U>
  inline wdBlobPtr(U* pPtr, wdUInt64 uiCount)
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

  /// \brief Initializes the wdBlobPtr to encapsulate the given array.
  template <size_t N>
  WD_ALWAYS_INLINE wdBlobPtr(ValueType (&staticArray)[N])
    : m_pPtr(staticArray)
    , m_uiCount(static_cast<wdUInt64>(N))
  {
  }

  /// \brief Initializes the wdBlobPtr to be a copy of \a other. No memory is allocated or copied.
  WD_ALWAYS_INLINE wdBlobPtr(const wdBlobPtr<T>& other)
    : m_pPtr(other.m_pPtr)
    , m_uiCount(other.m_uiCount)
  {
  }

  /// \brief Convert to const version.
  operator wdBlobPtr<const T>() const { return wdBlobPtr<const T>(static_cast<const T*>(GetPtr()), GetCount()); }

  /// \brief Copies the pointer and size of /a other. Does not allocate any data.
  WD_ALWAYS_INLINE void operator=(const wdBlobPtr<T>& other)
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

  WD_ALWAYS_INLINE void operator=(std::nullptr_t)
  {
    m_pPtr = nullptr;
    m_uiCount = 0;
  }

  /// \brief Returns the pointer to the array.
  WD_ALWAYS_INLINE PointerType GetPtr() const { return m_pPtr; }

  /// \brief Returns the pointer to the array.
  WD_ALWAYS_INLINE PointerType GetPtr() { return m_pPtr; }

  /// \brief Returns the pointer behind the last element of the array
  WD_ALWAYS_INLINE PointerType GetEndPtr() { return m_pPtr + m_uiCount; }

  /// \brief Returns the pointer behind the last element of the array
  WD_ALWAYS_INLINE PointerType GetEndPtr() const { return m_pPtr + m_uiCount; }

  /// \brief Returns whether the array is empty.
  WD_ALWAYS_INLINE bool IsEmpty() const { return GetCount() == 0; }

  /// \brief Returns the number of elements in the array.
  WD_ALWAYS_INLINE wdUInt64 GetCount() const { return m_uiCount; }

  /// \brief Creates a sub-array from this array.
  WD_FORCE_INLINE wdBlobPtr<T> GetSubArray(wdUInt64 uiStart, wdUInt64 uiCount) const // [tested]
  {
    WD_ASSERT_DEV(
      uiStart + uiCount <= GetCount(), "uiStart+uiCount ({0}) has to be smaller or equal than the count ({1}).", uiStart + uiCount, GetCount());
    return wdBlobPtr<T>(GetPtr() + uiStart, uiCount);
  }

  /// \brief Creates a sub-array from this array.
  /// \note \code ap.GetSubArray(i) \endcode is equivalent to \code ap.GetSubArray(i, ap.GetCount() - i) \endcode.
  WD_FORCE_INLINE wdBlobPtr<T> GetSubArray(wdUInt64 uiStart) const // [tested]
  {
    WD_ASSERT_DEV(uiStart <= GetCount(), "uiStart ({0}) has to be smaller or equal than the count ({1}).", uiStart, GetCount());
    return wdBlobPtr<T>(GetPtr() + uiStart, GetCount() - uiStart);
  }

  /// \brief Reinterprets this array as a byte array.
  WD_ALWAYS_INLINE wdBlobPtr<const ByteType> ToByteBlob() const
  {
    return wdBlobPtr<const ByteType>(reinterpret_cast<const ByteType*>(GetPtr()), GetCount() * sizeof(T));
  }

  /// \brief Reinterprets this array as a byte array.
  WD_ALWAYS_INLINE wdBlobPtr<ByteType> ToByteBlob() { return wdBlobPtr<ByteType>(reinterpret_cast<ByteType*>(GetPtr()), GetCount() * sizeof(T)); }

  /// \brief Cast an BlobPtr to an BlobPtr to a different, but same size, type
  template <typename U>
  WD_ALWAYS_INLINE wdBlobPtr<U> Cast()
  {
    static_assert(sizeof(T) == sizeof(U), "Can only cast with equivalent element size.");
    return wdBlobPtr<U>(reinterpret_cast<U*>(GetPtr()), GetCount());
  }

  /// \brief Cast an BlobPtr to an BlobPtr to a different, but same size, type
  template <typename U>
  WD_ALWAYS_INLINE wdBlobPtr<const U> Cast() const
  {
    static_assert(sizeof(T) == sizeof(U), "Can only cast with equivalent element size.");
    return wdBlobPtr<const U>(reinterpret_cast<const U*>(GetPtr()), GetCount());
  }

  /// \brief Index access.
  WD_FORCE_INLINE const ValueType& operator[](wdUInt64 uiIndex) const // [tested]
  {
    WD_ASSERT_DEV(uiIndex < GetCount(), "Cannot access element {0}, the array only holds {1} elements.", uiIndex, GetCount());
    return *static_cast<const ValueType*>(GetPtr() + uiIndex);
  }

  /// \brief Index access.
  WD_FORCE_INLINE ValueType& operator[](wdUInt64 uiIndex) // [tested]
  {
    WD_ASSERT_DEV(uiIndex < GetCount(), "Cannot access element {0}, the array only holds {1} elements.", uiIndex, GetCount());
    return *static_cast<ValueType*>(GetPtr() + uiIndex);
  }

  /// \brief Compares the two arrays for equality.
  inline bool operator==(const wdBlobPtr<const T>& other) const // [tested]
  {
    if (GetCount() != other.GetCount())
      return false;

    if (GetPtr() == other.GetPtr())
      return true;

    return wdMemoryUtils::IsEqual(static_cast<const ValueType*>(GetPtr()), static_cast<const ValueType*>(other.GetPtr()), static_cast<size_t>(GetCount()));
  }

  /// \brief Compares the two arrays for inequality.
  WD_ALWAYS_INLINE bool operator!=(const wdBlobPtr<const T>& other) const // [tested]
  {
    return !(*this == other);
  }

  /// \brief Copies the data from \a other into this array. The arrays must have the exact same size.
  inline void CopyFrom(const wdBlobPtr<const T>& other) // [tested]
  {
    WD_ASSERT_DEV(GetCount() == other.GetCount(), "Count for copy does not match. Target has {0} elements, source {1} elements", GetCount(), other.GetCount());

    wdMemoryUtils::Copy(static_cast<ValueType*>(GetPtr()), static_cast<const ValueType*>(other.GetPtr()), static_cast<size_t>(GetCount()));
  }

  WD_ALWAYS_INLINE void Swap(wdBlobPtr<T>& other)
  {
    wdMath::Swap(m_pPtr, other.m_pPtr);
    wdMath::Swap(m_uiCount, other.m_uiCount);
  }

  typedef const T* const_iterator;
  typedef const_reverse_pointer_iterator<T> const_reverse_iterator;
  typedef T* iterator;
  typedef reverse_pointer_iterator<T> reverse_iterator;

private:
  PointerType m_pPtr;
  wdUInt64 m_uiCount;
};

//////////////////////////////////////////////////////////////////////////

using wdByteBlobPtr = wdBlobPtr<wdUInt8>;
using wdConstByteBlobPtr = wdBlobPtr<const wdUInt8>;

//////////////////////////////////////////////////////////////////////////

/// \brief Helper function to create wdBlobPtr from a pointer of some type and a count.
template <typename T>
WD_ALWAYS_INLINE wdBlobPtr<T> wdMakeBlobPtr(T* pPtr, wdUInt64 uiCount)
{
  return wdBlobPtr<T>(pPtr, uiCount);
}

/// \brief Helper function to create wdBlobPtr from a static array the a size known at compile-time.
template <typename T, wdUInt64 N>
WD_ALWAYS_INLINE wdBlobPtr<T> wdMakeBlobPtr(T (&staticArray)[N])
{
  return wdBlobPtr<T>(staticArray);
}

/// \brief Helper function to create wdConstByteBlobPtr from a pointer of some type and a count.
template <typename T>
WD_ALWAYS_INLINE wdConstByteBlobPtr wdMakeByteBlobPtr(const T* pPtr, wdUInt32 uiCount)
{
  return wdConstByteBlobPtr(static_cast<const wdUInt8*>(pPtr), uiCount * sizeof(T));
}

/// \brief Helper function to create wdByteBlobPtr from a pointer of some type and a count.
template <typename T>
WD_ALWAYS_INLINE wdByteBlobPtr wdMakeByteBlobPtr(T* pPtr, wdUInt32 uiCount)
{
  return wdByteBlobPtr(reinterpret_cast<wdUInt8*>(pPtr), uiCount * sizeof(T));
}

/// \brief Helper function to create wdByteBlobPtr from a void pointer and a count.
WD_ALWAYS_INLINE wdByteBlobPtr wdMakeByteBlobPtr(void* pPtr, wdUInt32 uiBytes)
{
  return wdByteBlobPtr(reinterpret_cast<wdUInt8*>(pPtr), uiBytes);
}

/// \brief Helper function to create wdConstByteBlobPtr from a const void pointer and a count.
WD_ALWAYS_INLINE wdConstByteBlobPtr wdMakeByteBlobPtr(const void* pPtr, wdUInt32 uiBytes)
{
  return wdConstByteBlobPtr(static_cast<const wdUInt8*>(pPtr), uiBytes);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
typename wdBlobPtr<T>::iterator begin(wdBlobPtr<T>& in_container)
{
  return in_container.GetPtr();
}

template <typename T>
typename wdBlobPtr<T>::const_iterator begin(const wdBlobPtr<T>& container)
{
  return container.GetPtr();
}

template <typename T>
typename wdBlobPtr<T>::const_iterator cbegin(const wdBlobPtr<T>& container)
{
  return container.GetPtr();
}

template <typename T>
typename wdBlobPtr<T>::reverse_iterator rbegin(wdBlobPtr<T>& in_container)
{
  return typename wdBlobPtr<T>::reverse_iterator(in_container.GetPtr() + in_container.GetCount() - 1);
}

template <typename T>
typename wdBlobPtr<T>::const_reverse_iterator rbegin(const wdBlobPtr<T>& container)
{
  return typename wdBlobPtr<T>::const_reverse_iterator(container.GetPtr() + container.GetCount() - 1);
}

template <typename T>
typename wdBlobPtr<T>::const_reverse_iterator crbegin(const wdBlobPtr<T>& container)
{
  return typename wdBlobPtr<T>::const_reverse_iterator(container.GetPtr() + container.GetCount() - 1);
}

template <typename T>
typename wdBlobPtr<T>::iterator end(wdBlobPtr<T>& in_container)
{
  return in_container.GetPtr() + in_container.GetCount();
}

template <typename T>
typename wdBlobPtr<T>::const_iterator end(const wdBlobPtr<T>& container)
{
  return container.GetPtr() + container.GetCount();
}

template <typename T>
typename wdBlobPtr<T>::const_iterator cend(const wdBlobPtr<T>& container)
{
  return container.GetPtr() + container.GetCount();
}

template <typename T>
typename wdBlobPtr<T>::reverse_iterator rend(wdBlobPtr<T>& in_container)
{
  return typename wdBlobPtr<T>::reverse_iterator(in_container.GetPtr() - 1);
}

template <typename T>
typename wdBlobPtr<T>::const_reverse_iterator rend(const wdBlobPtr<T>& container)
{
  return typename wdBlobPtr<T>::const_reverse_iterator(container.GetPtr() - 1);
}

template <typename T>
typename wdBlobPtr<T>::const_reverse_iterator crend(const wdBlobPtr<T>& container)
{
  return typename wdBlobPtr<T>::const_reverse_iterator(container.GetPtr() - 1);
}

/// \brief wdBlob allows to store simple binary data larger than 4GB.
/// This storage class is used by wdImage to allow processing of large textures for example.
/// In the current implementation the start of the allocated memory is guaranteed to be 64 byte aligned.
class WD_FOUNDATION_DLL wdBlob
{
public:
  WD_DECLARE_MEM_RELOCATABLE_TYPE();

  /// \brief Default constructor. Does not allocate any memory.
  wdBlob();

  /// \brief Move constructor. Moves the storage pointer from the other blob to this blob.
  wdBlob(wdBlob&& other);

  /// \brief Move assignment. Moves the storage pointer from the other blob to this blob.
  void operator=(wdBlob&& rhs);

  /// \brief Default destructor. Will call Clear() to deallocate the memory.
  ~wdBlob();

  /// \brief Sets the blob to the content of pSource.
  /// This will allocate the necessary memory if needed and then copy uiSize bytes from pSource.
  void SetFrom(void* pSource, wdUInt64 uiSize);

  /// \brief Deallocates the memory allocated by this instance.
  void Clear();

  /// \brief Allocates uiCount bytes for storage in this object. The bytes will have undefined content.
  void SetCountUninitialized(wdUInt64 uiCount);

  /// \brief Convenience method to clear the content of the blob to all 0 bytes.
  void ZeroFill();

  /// \brief Returns a blob pointer to the blob data, or an empty blob pointer if the blob is empty.
  template <typename T>
  wdBlobPtr<T> GetBlobPtr()
  {
    return wdBlobPtr<T>(static_cast<T*>(m_pStorage), m_uiSize);
  }

  /// \brief Returns a blob pointer to the blob data, or an empty blob pointer if the blob is empty.
  template <typename T>
  wdBlobPtr<const T> GetBlobPtr() const
  {
    return wdBlobPtr<const T>(static_cast<T*>(m_pStorage), m_uiSize);
  }

  /// \brief Returns a blob pointer to the blob data, or an empty blob pointer if the blob is empty.
  wdByteBlobPtr GetByteBlobPtr() { return wdByteBlobPtr(reinterpret_cast<wdUInt8*>(m_pStorage), m_uiSize); }

  /// \brief Returns a blob pointer to the blob data, or an empty blob pointer if the blob is empty.
  wdConstByteBlobPtr GetByteBlobPtr() const { return wdConstByteBlobPtr(reinterpret_cast<const wdUInt8*>(m_pStorage), m_uiSize); }

private:
  void* m_pStorage = nullptr;
  wdUInt64 m_uiSize = 0;
};
