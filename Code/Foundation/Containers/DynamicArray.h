#pragma once

#include <Foundation/Containers/ArrayBase.h>
#include <Foundation/Memory/AllocatorWrapper.h>
#include <Foundation/Types/PointerWithFlags.h>

/// \brief Implementation of a dynamically growing array.
///
/// Best-case performance for the PushBack operation is O(1) if the nsDynamicArray doesn't need to be expanded.
/// In the worst case, PushBack is O(n).
/// Look-up is guaranteed to always be O(1).
template <typename T>
class nsDynamicArrayBase : public nsArrayBase<T, nsDynamicArrayBase<T>>
{
protected:
  /// \brief Creates an empty array. Does not allocate any data yet.
  explicit nsDynamicArrayBase(nsAllocator* pAllocator);                                 // [tested]

  nsDynamicArrayBase(T* pInplaceStorage, nsUInt32 uiCapacity, nsAllocator* pAllocator); // [tested]

  /// \brief Creates a copy of the given array.
  nsDynamicArrayBase(const nsDynamicArrayBase<T>& other, nsAllocator* pAllocator); // [tested]

  /// \brief Moves the given array into this one.
  nsDynamicArrayBase(nsDynamicArrayBase<T>&& other, nsAllocator* pAllocator); // [tested]

  /// \brief Creates a copy of the given array.
  nsDynamicArrayBase(const nsArrayPtr<const T>& other, nsAllocator* pAllocator); // [tested]

  /// \brief Destructor.
  ~nsDynamicArrayBase(); // [tested]

  /// \brief Copies the data from some other contiguous array into this one.
  void operator=(const nsDynamicArrayBase<T>& rhs); // [tested]

  /// \brief Moves the data from some other contiguous array into this one.
  void operator=(nsDynamicArrayBase<T>&& rhs) noexcept; // [tested]

  T* GetElementsPtr();
  const T* GetElementsPtr() const;

  friend class nsArrayBase<T, nsDynamicArrayBase<T>>;

public:
  /// \brief Expands the array so it can at least store the given capacity.
  void Reserve(nsUInt32 uiCapacity); // [tested]

  /// \brief Tries to compact the array to avoid wasting memory. The resulting capacity is at least 'GetCount' (no elements get removed). Will
  /// deallocate all data, if the array is empty.
  void Compact(); // [tested]

  /// \brief Returns the allocator that is used by this instance.
  nsAllocator* GetAllocator() const { return const_cast<nsAllocator*>(m_pAllocator.GetPtr()); }

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  nsUInt64 GetHeapMemoryUsage() const; // [tested]

  /// \brief swaps the contents of this array with another one
  void Swap(nsDynamicArrayBase<T>& other); // [tested]

private:
  enum Storage
  {
    Owned = 0,
    External = 1
  };

  nsPointerWithFlags<nsAllocator, 1> m_pAllocator;

  enum
  {
    CAPACITY_ALIGNMENT = 16
  };

  void SetCapacity(nsUInt32 uiCapacity);
};

/// \brief \see nsDynamicArrayBase
template <typename T, typename AllocatorWrapper = nsDefaultAllocatorWrapper>
class nsDynamicArray : public nsDynamicArrayBase<T>
{
public:
  NS_DECLARE_MEM_RELOCATABLE_TYPE();


  nsDynamicArray();
  explicit nsDynamicArray(nsAllocator* pAllocator);

  nsDynamicArray(const nsDynamicArray<T, AllocatorWrapper>& other);
  nsDynamicArray(const nsDynamicArrayBase<T>& other);
  explicit nsDynamicArray(const nsArrayPtr<const T>& other);

  nsDynamicArray(nsDynamicArray<T, AllocatorWrapper>&& other);
  nsDynamicArray(nsDynamicArrayBase<T>&& other);

  void operator=(const nsDynamicArray<T, AllocatorWrapper>& rhs);
  void operator=(const nsDynamicArrayBase<T>& rhs);
  void operator=(const nsArrayPtr<const T>& rhs);

  void operator=(nsDynamicArray<T, AllocatorWrapper>&& rhs) noexcept;
  void operator=(nsDynamicArrayBase<T>&& rhs) noexcept;

protected:
  nsDynamicArray(T* pInplaceStorage, nsUInt32 uiCapacity, nsAllocator* pAllocator)
    : nsDynamicArrayBase<T>(pInplaceStorage, uiCapacity, pAllocator)
  {
  }
};

/// Overload of nsMakeArrayPtr for const dynamic arrays of pointer pointing to const type.
template <typename T, typename AllocatorWrapper>
nsArrayPtr<const T* const> nsMakeArrayPtr(const nsDynamicArray<T*, AllocatorWrapper>& dynArray);

/// Overload of nsMakeArrayPtr for const dynamic arrays.
template <typename T, typename AllocatorWrapper>
nsArrayPtr<const T> nsMakeArrayPtr(const nsDynamicArray<T, AllocatorWrapper>& dynArray);

/// Overload of nsMakeArrayPtr for dynamic arrays.
template <typename T, typename AllocatorWrapper>
nsArrayPtr<T> nsMakeArrayPtr(nsDynamicArray<T, AllocatorWrapper>& in_dynArray);


NS_CHECK_AT_COMPILETIME_MSG(nsGetTypeClass<nsDynamicArray<int>>::value == 2, "dynamic array is not memory relocatable");

#include <Foundation/Containers/Implementation/DynamicArray_inl.h>
