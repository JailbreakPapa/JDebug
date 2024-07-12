
#pragma once

#include <Foundation/Basics.h>

/// \brief This class encapsulates a blob's storage and it's size. It is recommended to use this class instead of directly working on the void* of the
/// blob.
///
/// No data is deallocated at destruction, the nsBlobPtr only allows for easier access.
template <typename T>
class nsBlobPtr
{
public:
  NS_DECLARE_POD_TYPE();

  static_assert(!std::is_same_v<T, void>, "nsBlobPtr<void> is not allowed (anymore)");
  static_assert(!std::is_same_v<T, const void>, "nsBlobPtr<void> is not allowed (anymore)");

  using ByteType = typename nsArrayPtrDetail::ByteTypeHelper<T>::type;
  using ValueType = T;
  using PointerType = T*;

  /// \brief Initializes the nsBlobPtr to be empty.
  nsBlobPtr() = default;

  /// \brief Initializes the nsBlobPtr with the given pointer and number of elements. No memory is allocated or copied.
  template <typename U>
  inline nsBlobPtr(U* pPtr, nsUInt64 uiCount)
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

  /// \brief Initializes the nsBlobPtr to encapsulate the given array.
  template <size_t N>
  NS_ALWAYS_INLINE nsBlobPtr(ValueType (&staticArray)[N])
    : m_pPtr(staticArray)
    , m_uiCount(static_cast<nsUInt64>(N))
  {
  }

  /// \brief Initializes the nsBlobPtr to be a copy of \a other. No memory is allocated or copied.
  NS_ALWAYS_INLINE nsBlobPtr(const nsBlobPtr<T>& other)
    : m_pPtr(other.m_pPtr)
    , m_uiCount(other.m_uiCount)
  {
  }

  /// \brief Convert to const version.
  operator nsBlobPtr<const T>() const { return nsBlobPtr<const T>(static_cast<const T*>(GetPtr()), GetCount()); }

  /// \brief Copies the pointer and size of /a other. Does not allocate any data.
  NS_ALWAYS_INLINE void operator=(const nsBlobPtr<T>& other)
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

  NS_ALWAYS_INLINE void operator=(std::nullptr_t)
  {
    m_pPtr = nullptr;
    m_uiCount = 0;
  }

  /// \brief Returns the pointer to the array.
  NS_ALWAYS_INLINE PointerType GetPtr() const { return m_pPtr; }

  /// \brief Returns the pointer to the array.
  NS_ALWAYS_INLINE PointerType GetPtr() { return m_pPtr; }

  /// \brief Returns the pointer behind the last element of the array
  NS_ALWAYS_INLINE PointerType GetEndPtr() { return m_pPtr + m_uiCount; }

  /// \brief Returns the pointer behind the last element of the array
  NS_ALWAYS_INLINE PointerType GetEndPtr() const { return m_pPtr + m_uiCount; }

  /// \brief Returns whether the array is empty.
  NS_ALWAYS_INLINE bool IsEmpty() const { return GetCount() == 0; }

  /// \brief Returns the number of elements in the array.
  NS_ALWAYS_INLINE nsUInt64 GetCount() const { return m_uiCount; }

  /// \brief Creates a sub-array from this array.
  NS_FORCE_INLINE nsBlobPtr<T> GetSubArray(nsUInt64 uiStart, nsUInt64 uiCount) const // [tested]
  {
    NS_ASSERT_DEV(
      uiStart + uiCount <= GetCount(), "uiStart+uiCount ({0}) has to be smaller or equal than the count ({1}).", uiStart + uiCount, GetCount());
    return nsBlobPtr<T>(GetPtr() + uiStart, uiCount);
  }

  /// \brief Creates a sub-array from this array.
  /// \note \code ap.GetSubArray(i) \endcode is equivalent to \code ap.GetSubArray(i, ap.GetCount() - i) \endcode.
  NS_FORCE_INLINE nsBlobPtr<T> GetSubArray(nsUInt64 uiStart) const // [tested]
  {
    NS_ASSERT_DEV(uiStart <= GetCount(), "uiStart ({0}) has to be smaller or equal than the count ({1}).", uiStart, GetCount());
    return nsBlobPtr<T>(GetPtr() + uiStart, GetCount() - uiStart);
  }

  /// \brief Reinterprets this array as a byte array.
  NS_ALWAYS_INLINE nsBlobPtr<const ByteType> ToByteBlob() const
  {
    return nsBlobPtr<const ByteType>(reinterpret_cast<const ByteType*>(GetPtr()), GetCount() * sizeof(T));
  }

  /// \brief Reinterprets this array as a byte array.
  NS_ALWAYS_INLINE nsBlobPtr<ByteType> ToByteBlob() { return nsBlobPtr<ByteType>(reinterpret_cast<ByteType*>(GetPtr()), GetCount() * sizeof(T)); }

  /// \brief Cast an BlobPtr to an BlobPtr to a different, but same size, type
  template <typename U>
  NS_ALWAYS_INLINE nsBlobPtr<U> Cast()
  {
    static_assert(sizeof(T) == sizeof(U), "Can only cast with equivalent element size.");
    return nsBlobPtr<U>(reinterpret_cast<U*>(GetPtr()), GetCount());
  }

  /// \brief Cast an BlobPtr to an BlobPtr to a different, but same size, type
  template <typename U>
  NS_ALWAYS_INLINE nsBlobPtr<const U> Cast() const
  {
    static_assert(sizeof(T) == sizeof(U), "Can only cast with equivalent element size.");
    return nsBlobPtr<const U>(reinterpret_cast<const U*>(GetPtr()), GetCount());
  }

  /// \brief Index access.
  NS_FORCE_INLINE const ValueType& operator[](nsUInt64 uiIndex) const // [tested]
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(), "Cannot access element {0}, the array only holds {1} elements.", uiIndex, GetCount());
    return *static_cast<const ValueType*>(GetPtr() + uiIndex);
  }

  /// \brief Index access.
  NS_FORCE_INLINE ValueType& operator[](nsUInt64 uiIndex) // [tested]
  {
    NS_ASSERT_DEBUG(uiIndex < GetCount(), "Cannot access element {0}, the array only holds {1} elements.", uiIndex, GetCount());
    return *static_cast<ValueType*>(GetPtr() + uiIndex);
  }

  /// \brief Compares the two arrays for equality.
  inline bool operator==(const nsBlobPtr<const T>& other) const // [tested]
  {
    if (GetCount() != other.GetCount())
      return false;

    if (GetPtr() == other.GetPtr())
      return true;

    return nsMemoryUtils::IsEqual(static_cast<const ValueType*>(GetPtr()), static_cast<const ValueType*>(other.GetPtr()), static_cast<size_t>(GetCount()));
  }

  /// \brief Compares the two arrays for inequality.
  NS_ALWAYS_INLINE bool operator!=(const nsBlobPtr<const T>& other) const // [tested]
  {
    return !(*this == other);
  }

  /// \brief Copies the data from \a other into this array. The arrays must have the exact same size.
  inline void CopyFrom(const nsBlobPtr<const T>& other) // [tested]
  {
    NS_ASSERT_DEV(GetCount() == other.GetCount(), "Count for copy does not match. Target has {0} elements, source {1} elements", GetCount(), other.GetCount());

    nsMemoryUtils::Copy(static_cast<ValueType*>(GetPtr()), static_cast<const ValueType*>(other.GetPtr()), static_cast<size_t>(GetCount()));
  }

  NS_ALWAYS_INLINE void Swap(nsBlobPtr<T>& other)
  {
    nsMath::Swap(m_pPtr, other.m_pPtr);
    nsMath::Swap(m_uiCount, other.m_uiCount);
  }

  using const_iterator = const T*;
  using const_reverse_iterator = const_reverse_pointer_iterator<T>;
  using iterator = T*;
  using reverse_iterator = reverse_pointer_iterator<T>;

private:
  PointerType m_pPtr = nullptr;
  nsUInt64 m_uiCount = 0u;
};

//////////////////////////////////////////////////////////////////////////

using nsByteBlobPtr = nsBlobPtr<nsUInt8>;
using nsConstByteBlobPtr = nsBlobPtr<const nsUInt8>;

//////////////////////////////////////////////////////////////////////////

/// \brief Helper function to create nsBlobPtr from a pointer of some type and a count.
template <typename T>
NS_ALWAYS_INLINE nsBlobPtr<T> nsMakeBlobPtr(T* pPtr, nsUInt64 uiCount)
{
  return nsBlobPtr<T>(pPtr, uiCount);
}

/// \brief Helper function to create nsBlobPtr from a static array the a size known at compile-time.
template <typename T, nsUInt64 N>
NS_ALWAYS_INLINE nsBlobPtr<T> nsMakeBlobPtr(T (&staticArray)[N])
{
  return nsBlobPtr<T>(staticArray);
}

/// \brief Helper function to create nsConstByteBlobPtr from a pointer of some type and a count.
template <typename T>
NS_ALWAYS_INLINE nsConstByteBlobPtr nsMakeByteBlobPtr(const T* pPtr, nsUInt32 uiCount)
{
  return nsConstByteBlobPtr(static_cast<const nsUInt8*>(pPtr), uiCount * sizeof(T));
}

/// \brief Helper function to create nsByteBlobPtr from a pointer of some type and a count.
template <typename T>
NS_ALWAYS_INLINE nsByteBlobPtr nsMakeByteBlobPtr(T* pPtr, nsUInt32 uiCount)
{
  return nsByteBlobPtr(reinterpret_cast<nsUInt8*>(pPtr), uiCount * sizeof(T));
}

/// \brief Helper function to create nsByteBlobPtr from a void pointer and a count.
NS_ALWAYS_INLINE nsByteBlobPtr nsMakeByteBlobPtr(void* pPtr, nsUInt32 uiBytes)
{
  return nsByteBlobPtr(reinterpret_cast<nsUInt8*>(pPtr), uiBytes);
}

/// \brief Helper function to create nsConstByteBlobPtr from a const void pointer and a count.
NS_ALWAYS_INLINE nsConstByteBlobPtr nsMakeByteBlobPtr(const void* pPtr, nsUInt32 uiBytes)
{
  return nsConstByteBlobPtr(static_cast<const nsUInt8*>(pPtr), uiBytes);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
typename nsBlobPtr<T>::iterator begin(nsBlobPtr<T>& in_container)
{
  return in_container.GetPtr();
}

template <typename T>
typename nsBlobPtr<T>::const_iterator begin(const nsBlobPtr<T>& container)
{
  return container.GetPtr();
}

template <typename T>
typename nsBlobPtr<T>::const_iterator cbegin(const nsBlobPtr<T>& container)
{
  return container.GetPtr();
}

template <typename T>
typename nsBlobPtr<T>::reverse_iterator rbegin(nsBlobPtr<T>& in_container)
{
  return typename nsBlobPtr<T>::reverse_iterator(in_container.GetPtr() + in_container.GetCount() - 1);
}

template <typename T>
typename nsBlobPtr<T>::const_reverse_iterator rbegin(const nsBlobPtr<T>& container)
{
  return typename nsBlobPtr<T>::const_reverse_iterator(container.GetPtr() + container.GetCount() - 1);
}

template <typename T>
typename nsBlobPtr<T>::const_reverse_iterator crbegin(const nsBlobPtr<T>& container)
{
  return typename nsBlobPtr<T>::const_reverse_iterator(container.GetPtr() + container.GetCount() - 1);
}

template <typename T>
typename nsBlobPtr<T>::iterator end(nsBlobPtr<T>& in_container)
{
  return in_container.GetPtr() + in_container.GetCount();
}

template <typename T>
typename nsBlobPtr<T>::const_iterator end(const nsBlobPtr<T>& container)
{
  return container.GetPtr() + container.GetCount();
}

template <typename T>
typename nsBlobPtr<T>::const_iterator cend(const nsBlobPtr<T>& container)
{
  return container.GetPtr() + container.GetCount();
}

template <typename T>
typename nsBlobPtr<T>::reverse_iterator rend(nsBlobPtr<T>& in_container)
{
  return typename nsBlobPtr<T>::reverse_iterator(in_container.GetPtr() - 1);
}

template <typename T>
typename nsBlobPtr<T>::const_reverse_iterator rend(const nsBlobPtr<T>& container)
{
  return typename nsBlobPtr<T>::const_reverse_iterator(container.GetPtr() - 1);
}

template <typename T>
typename nsBlobPtr<T>::const_reverse_iterator crend(const nsBlobPtr<T>& container)
{
  return typename nsBlobPtr<T>::const_reverse_iterator(container.GetPtr() - 1);
}

/// \brief nsBlob allows to store simple binary data larger than 4GB.
/// This storage class is used by nsImage to allow processing of large textures for example.
/// In the current implementation the start of the allocated memory is guaranteed to be 64 byte aligned.
class NS_FOUNDATION_DLL nsBlob
{
public:
  NS_DECLARE_MEM_RELOCATABLE_TYPE();

  /// \brief Default constructor. Does not allocate any memory.
  nsBlob();

  /// \brief Move constructor. Moves the storage pointer from the other blob to this blob.
  nsBlob(nsBlob&& other);

  /// \brief Move assignment. Moves the storage pointer from the other blob to this blob.
  void operator=(nsBlob&& rhs);

  /// \brief Default destructor. Will call Clear() to deallocate the memory.
  ~nsBlob();

  /// \brief Sets the blob to the content of pSource.
  /// This will allocate the necessary memory if needed and then copy uiSize bytes from pSource.
  void SetFrom(const void* pSource, nsUInt64 uiSize);

  /// \brief Deallocates the memory allocated by this instance.
  void Clear();

  /// \bried Is data blob empty
  bool IsEmpty() const;

  /// \brief Allocates uiCount bytes for storage in this object. The bytes will have undefined content.
  void SetCountUninitialized(nsUInt64 uiCount);

  /// \brief Convenience method to clear the content of the blob to all 0 bytes.
  void ZeroFill();

  /// \brief Returns a blob pointer to the blob data, or an empty blob pointer if the blob is empty.
  template <typename T>
  nsBlobPtr<T> GetBlobPtr()
  {
    return nsBlobPtr<T>(static_cast<T*>(m_pStorage), m_uiSize);
  }

  /// \brief Returns a blob pointer to the blob data, or an empty blob pointer if the blob is empty.
  template <typename T>
  nsBlobPtr<const T> GetBlobPtr() const
  {
    return nsBlobPtr<const T>(static_cast<T*>(m_pStorage), m_uiSize);
  }

  /// \brief Returns a blob pointer to the blob data, or an empty blob pointer if the blob is empty.
  nsByteBlobPtr GetByteBlobPtr() { return nsByteBlobPtr(reinterpret_cast<nsUInt8*>(m_pStorage), m_uiSize); }

  /// \brief Returns a blob pointer to the blob data, or an empty blob pointer if the blob is empty.
  nsConstByteBlobPtr GetByteBlobPtr() const { return nsConstByteBlobPtr(reinterpret_cast<const nsUInt8*>(m_pStorage), m_uiSize); }

private:
  void* m_pStorage = nullptr;
  nsUInt64 m_uiSize = 0;
};
