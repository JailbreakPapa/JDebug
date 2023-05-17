#pragma once

#include <Foundation/Algorithm/Sorting.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Memory/AllocatorWrapper.h>
#include <Foundation/Types/ArrayPtr.h>

constexpr wdUInt32 wdSmallInvalidIndex = 0xFFFF;

/// \brief Implementation of a dynamically growing array with in-place storage and small memory overhead.
///
/// Best-case performance for the PushBack operation is in O(1) if the wdHybridArray does not need to be expanded.
/// In the worst case, PushBack is in O(n).
/// Look-up is guaranteed to always be in O(1).
template <typename T, wdUInt16 Size>
class wdSmallArrayBase
{
public:
  // Only if the stored type is either POD or relocatable the hybrid array itself is also relocatable.
  WD_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T);

  wdSmallArrayBase();                                                                    // [tested]
  wdSmallArrayBase(const wdSmallArrayBase<T, Size>& other, wdAllocatorBase* pAllocator); // [tested]
  wdSmallArrayBase(const wdArrayPtr<const T>& other, wdAllocatorBase* pAllocator);       // [tested]
  wdSmallArrayBase(wdSmallArrayBase<T, Size>&& other, wdAllocatorBase* pAllocator);      // [tested]

  ~wdSmallArrayBase(); // [tested]

  // Can't use regular assignment operators since we need to pass an allocator. Use CopyFrom or MoveFrom methods instead.
  void operator=(const wdSmallArrayBase<T, Size>& rhs) = delete;
  void operator=(wdSmallArrayBase<T, Size>&& rhs) = delete;

  /// \brief Copies the data from some other array into this one.
  void CopyFrom(const wdArrayPtr<const T>& other, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Moves the data from some other array into this one.
  void MoveFrom(wdSmallArrayBase<T, Size>&& other, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Conversion to const wdArrayPtr.
  operator wdArrayPtr<const T>() const; // [tested]

  /// \brief Conversion to wdArrayPtr.
  operator wdArrayPtr<T>(); // [tested]

  /// \brief Compares this array to another contiguous array type.
  bool operator==(const wdSmallArrayBase<T, Size>& rhs) const; // [tested]
  bool operator==(const wdArrayPtr<const T>& rhs) const;       // [tested]

  /// \brief Compares this array to another contiguous array type.
  bool operator!=(const wdSmallArrayBase<T, Size>& rhs) const; // [tested]
  bool operator!=(const wdArrayPtr<const T>& rhs) const;       // [tested]

  /// \brief Returns the element at the given index. Does bounds checks in debug builds.
  const T& operator[](wdUInt32 uiIndex) const; // [tested]

  /// \brief Returns the element at the given index. Does bounds checks in debug builds.
  T& operator[](wdUInt32 uiIndex); // [tested]

  /// \brief Resizes the array to have exactly uiCount elements. Default constructs extra elements if the array is grown.
  void SetCount(wdUInt16 uiCount, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Resizes the array to have exactly uiCount elements. Constructs all new elements by copying the FillValue.
  void SetCount(wdUInt16 uiCount, const T& fillValue, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Resizes the array to have exactly uiCount elements. Extra elements might be uninitialized.
  template <typename = void>                                                 // Template is used to only conditionally compile this function in when it is actually used.
  void SetCountUninitialized(wdUInt16 uiCount, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Ensures the container has at least \a uiCount elements. Ie. calls SetCount() if the container has fewer elements, does nothing
  /// otherwise.
  void EnsureCount(wdUInt16 uiCount, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Returns the number of active elements in the array.
  wdUInt32 GetCount() const; // [tested]

  /// \brief Returns true, if the array does not contain any elements.
  bool IsEmpty() const; // [tested]

  /// \brief Clears the array.
  void Clear(); // [tested]

  /// \brief Checks whether the given value can be found in the array. O(n) complexity.
  bool Contains(const T& value) const; // [tested]

  /// \brief Inserts value at index by shifting all following elements.
  void Insert(const T& value, wdUInt32 uiIndex, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Inserts value at index by shifting all following elements.
  void Insert(T&& value, wdUInt32 uiIndex, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Removes the first occurrence of value and fills the gap by shifting all following elements
  bool RemoveAndCopy(const T& value); // [tested]

  /// \brief Removes the first occurrence of value and fills the gap by swapping in the last element
  bool RemoveAndSwap(const T& value); // [tested]

  /// \brief Removes the element at index and fills the gap by shifting all following elements
  void RemoveAtAndCopy(wdUInt32 uiIndex, wdUInt32 uiNumElements = 1); // [tested]

  /// \brief Removes the element at index and fills the gap by swapping in the last element
  void RemoveAtAndSwap(wdUInt32 uiIndex, wdUInt32 uiNumElements = 1); // [tested]

  /// \brief Searches for the first occurrence of the given value and returns its index or wdInvalidIndex if not found.
  wdUInt32 IndexOf(const T& value, wdUInt32 uiStartIndex = 0) const; // [tested]

  /// \brief Searches for the last occurrence of the given value and returns its index or wdInvalidIndex if not found.
  wdUInt32 LastIndexOf(const T& value, wdUInt32 uiStartIndex = wdSmallInvalidIndex) const; // [tested]

  /// \brief Grows the array by one element and returns a reference to the newly created element.
  T& ExpandAndGetRef(wdAllocatorBase* pAllocator); // [tested]

  /// \brief Pushes value at the end of the array.
  void PushBack(const T& value, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Pushes value at the end of the array.
  void PushBack(T&& value, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Pushes value at the end of the array. Does NOT ensure capacity.
  void PushBackUnchecked(const T& value); // [tested]

  /// \brief Pushes value at the end of the array. Does NOT ensure capacity.
  void PushBackUnchecked(T&& value); // [tested]

  /// \brief Pushes all elements in range at the end of the array. Increases the capacity if necessary.
  void PushBackRange(const wdArrayPtr<const T>& range, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Removes count elements from the end of the array.
  void PopBack(wdUInt32 uiCountToRemove = 1); // [tested]

  /// \brief Returns the last element of the array.
  T& PeekBack(); // [tested]

  /// \brief Returns the last element of the array.
  const T& PeekBack() const; // [tested]

  /// \brief Sort with explicit comparer
  template <typename Comparer>
  void Sort(const Comparer& comparer); // [tested]

  /// \brief Sort with default comparer
  void Sort(); // [tested]

  /// \brief Returns a pointer to the array data, or nullptr if the array is empty.
  T* GetData();

  /// \brief Returns a pointer to the array data, or nullptr if the array is empty.
  const T* GetData() const;

  /// \brief Returns an array pointer to the array data, or an empty array pointer if the array is empty.
  wdArrayPtr<T> GetArrayPtr(); // [tested]

  /// \brief Returns an array pointer to the array data, or an empty array pointer if the array is empty.
  wdArrayPtr<const T> GetArrayPtr() const; // [tested]

  /// \brief Returns a byte array pointer to the array data, or an empty array pointer if the array is empty.
  wdArrayPtr<typename wdArrayPtr<T>::ByteType> GetByteArrayPtr(); // [tested]

  /// \brief Returns a byte array pointer to the array data, or an empty array pointer if the array is empty.
  wdArrayPtr<typename wdArrayPtr<const T>::ByteType> GetByteArrayPtr() const; // [tested]

  /// \brief Expands the array so it can at least store the given capacity.
  void Reserve(wdUInt16 uiCapacity, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Tries to compact the array to avoid wasting memory. The resulting capacity is at least 'GetCount' (no elements get removed). Will
  /// deallocate all data, if the array is empty.
  void Compact(wdAllocatorBase* pAllocator); // [tested]

  /// \brief Returns the reserved number of elements that the array can hold without reallocating.
  wdUInt32 GetCapacity() const { return m_uiCapacity; }

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  wdUInt64 GetHeapMemoryUsage() const; // [tested]

  using const_iterator = const T*;
  using const_reverse_iterator = const_reverse_pointer_iterator<T>;
  using iterator = T*;
  using reverse_iterator = reverse_pointer_iterator<T>;

  template <typename U>
  const U& GetUserData() const; // [tested]

  template <typename U>
  U& GetUserData(); // [tested]

protected:
  enum
  {
    CAPACITY_ALIGNMENT = 4
  };

  void SetCapacity(wdUInt16 uiCapacity, wdAllocatorBase* pAllocator);

  T* GetElementsPtr();
  const T* GetElementsPtr() const;

  wdUInt16 m_uiCount = 0;
  wdUInt16 m_uiCapacity = Size;

  wdUInt32 m_uiUserData = 0;

  union
  {
    struct alignas(WD_ALIGNMENT_OF(T))
    {
      wdUInt8 m_StaticData[Size * sizeof(T)];
    };

    T* m_pElements = nullptr;
  };
};

//////////////////////////////////////////////////////////////////////////

/// \brief \see wdSmallArrayBase
template <typename T, wdUInt16 Size, typename AllocatorWrapper = wdDefaultAllocatorWrapper>
class wdSmallArray : public wdSmallArrayBase<T, Size>
{
  using SUPER = wdSmallArrayBase<T, Size>;

public:
  // Only if the stored type is either POD or relocatable the hybrid array itself is also relocatable.
  WD_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T);

  wdSmallArray();

  wdSmallArray(const wdSmallArray<T, Size, AllocatorWrapper>& other);
  wdSmallArray(const wdArrayPtr<const T>& other);
  wdSmallArray(wdSmallArray<T, Size, AllocatorWrapper>&& other);

  ~wdSmallArray();

  void operator=(const wdSmallArray<T, Size, AllocatorWrapper>& rhs);
  void operator=(const wdArrayPtr<const T>& rhs);
  void operator=(wdSmallArray<T, Size, AllocatorWrapper>&& rhs) noexcept;

  void SetCount(wdUInt16 uiCount);                     // [tested]
  void SetCount(wdUInt16 uiCount, const T& fillValue); // [tested]
  void EnsureCount(wdUInt16 uiCount);                  // [tested]

  template <typename = void>
  void SetCountUninitialized(wdUInt16 uiCount); // [tested]

  void Insert(const T& value, wdUInt32 uiIndex); // [tested]
  void Insert(T&& value, wdUInt32 uiIndex);      // [tested]

  T& ExpandAndGetRef();                                 // [tested]
  void PushBack(const T& value);                        // [tested]
  void PushBack(T&& value);                             // [tested]
  void PushBackRange(const wdArrayPtr<const T>& range); // [tested]

  void Reserve(wdUInt16 uiCapacity);
  void Compact();
};

#include <Foundation/Containers/Implementation/SmallArray_inl.h>
