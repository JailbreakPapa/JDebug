#pragma once

#include <Foundation/Containers/ArrayBase.h>
#include <Foundation/Memory/AllocatorWrapper.h>
#include <Foundation/Types/PointerWithFlags.h>

/// \brief Implementation of a dynamically growing array.
///
/// Best-case performance for the PushBack operation is O(1) if the wdDynamicArray doesn't need to be expanded.
/// In the worst case, PushBack is O(n).
/// Look-up is guaranteed to always be O(1).
template <typename T>
class wdDynamicArrayBase : public wdArrayBase<T, wdDynamicArrayBase<T>>
{
protected:
  /// \brief Creates an empty array. Does not allocate any data yet.
  explicit wdDynamicArrayBase(wdAllocatorBase* pAllocator); // [tested]

  wdDynamicArrayBase(T* pInplaceStorage, wdUInt32 uiCapacity, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Creates a copy of the given array.
  wdDynamicArrayBase(const wdDynamicArrayBase<T>& other, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Moves the given array into this one.
  wdDynamicArrayBase(wdDynamicArrayBase<T>&& other, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Creates a copy of the given array.
  wdDynamicArrayBase(const wdArrayPtr<const T>& other, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Destructor.
  ~wdDynamicArrayBase(); // [tested]

  /// \brief Copies the data from some other contiguous array into this one.
  void operator=(const wdDynamicArrayBase<T>& rhs); // [tested]

  /// \brief Moves the data from some other contiguous array into this one.
  void operator=(wdDynamicArrayBase<T>&& rhs) noexcept; // [tested]

  T* GetElementsPtr();
  const T* GetElementsPtr() const;

  friend class wdArrayBase<T, wdDynamicArrayBase<T>>;

public:
  /// \brief Expands the array so it can at least store the given capacity.
  void Reserve(wdUInt32 uiCapacity); // [tested]

  /// \brief Tries to compact the array to avoid wasting memory. The resulting capacity is at least 'GetCount' (no elements get removed). Will
  /// deallocate all data, if the array is empty.
  void Compact(); // [tested]

  /// \brief Returns the allocator that is used by this instance.
  wdAllocatorBase* GetAllocator() const { return const_cast<wdAllocatorBase*>(m_pAllocator.GetPtr()); }

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  wdUInt64 GetHeapMemoryUsage() const; // [tested]

  /// \brief swaps the contents of this array with another one
  void Swap(wdDynamicArrayBase<T>& other); // [tested]

private:
  enum Storage
  {
    Owned = 0,
    External = 1
  };

  wdPointerWithFlags<wdAllocatorBase, 1> m_pAllocator;

  enum
  {
    CAPACITY_ALIGNMENT = 16
  };

  void SetCapacity(wdUInt32 uiCapacity);
};

/// \brief \see wdDynamicArrayBase
template <typename T, typename AllocatorWrapper = wdDefaultAllocatorWrapper>
class wdDynamicArray : public wdDynamicArrayBase<T>
{
public:
  WD_DECLARE_MEM_RELOCATABLE_TYPE();


  wdDynamicArray();
  explicit wdDynamicArray(wdAllocatorBase* pAllocator);

  wdDynamicArray(const wdDynamicArray<T, AllocatorWrapper>& other);
  wdDynamicArray(const wdDynamicArrayBase<T>& other);
  explicit wdDynamicArray(const wdArrayPtr<const T>& other);

  wdDynamicArray(wdDynamicArray<T, AllocatorWrapper>&& other);
  wdDynamicArray(wdDynamicArrayBase<T>&& other);

  void operator=(const wdDynamicArray<T, AllocatorWrapper>& rhs);
  void operator=(const wdDynamicArrayBase<T>& rhs);
  void operator=(const wdArrayPtr<const T>& rhs);

  void operator=(wdDynamicArray<T, AllocatorWrapper>&& rhs) noexcept;
  void operator=(wdDynamicArrayBase<T>&& rhs) noexcept;

protected:
  wdDynamicArray(T* pInplaceStorage, wdUInt32 uiCapacity, wdAllocatorBase* pAllocator)
    : wdDynamicArrayBase<T>(pInplaceStorage, uiCapacity, pAllocator)
  {
  }
};

/// Overload of wdMakeArrayPtr for const dynamic arrays of pointer pointing to const type.
template <typename T, typename AllocatorWrapper>
wdArrayPtr<const T* const> wdMakeArrayPtr(const wdDynamicArray<T*, AllocatorWrapper>& dynArray);

/// Overload of wdMakeArrayPtr for const dynamic arrays.
template <typename T, typename AllocatorWrapper>
wdArrayPtr<const T> wdMakeArrayPtr(const wdDynamicArray<T, AllocatorWrapper>& dynArray);

/// Overload of wdMakeArrayPtr for dynamic arrays.
template <typename T, typename AllocatorWrapper>
wdArrayPtr<T> wdMakeArrayPtr(wdDynamicArray<T, AllocatorWrapper>& in_dynArray);


WD_CHECK_AT_COMPILETIME_MSG(wdGetTypeClass<wdDynamicArray<int>>::value == 2, "dynamic array is not memory relocatable");

#include <Foundation/Containers/Implementation/DynamicArray_inl.h>
