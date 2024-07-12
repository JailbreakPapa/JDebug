#pragma once

#include <Foundation/Algorithm/Sorting.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Memory/AllocatorWrapper.h>
#include <Foundation/Types/ArrayPtr.h>

constexpr nsUInt32 nsSmallInvalidIndex = 0xFFFF;

/// \brief Implementation of a dynamically growing array with in-place storage and small memory overhead.
///
/// Best-case performance for the PushBack operation is in O(1) if the nsHybridArray does not need to be expanded.
/// In the worst case, PushBack is in O(n).
/// Look-up is guaranteed to always be in O(1).
template <typename T, nsUInt16 Size>
class nsSmallArrayBase
{
public:
  // Only if the stored type is either POD or relocatable the hybrid array itself is also relocatable.
  NS_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T);

  nsSmallArrayBase();                                                                // [tested]
  nsSmallArrayBase(const nsSmallArrayBase<T, Size>& other, nsAllocator* pAllocator); // [tested]
  nsSmallArrayBase(const nsArrayPtr<const T>& other, nsAllocator* pAllocator);       // [tested]
  nsSmallArrayBase(nsSmallArrayBase<T, Size>&& other, nsAllocator* pAllocator);      // [tested]

  ~nsSmallArrayBase();                                                               // [tested]

  // Can't use regular assignment operators since we need to pass an allocator. Use CopyFrom or MoveFrom methods instead.
  void operator=(const nsSmallArrayBase<T, Size>& rhs) = delete;
  void operator=(nsSmallArrayBase<T, Size>&& rhs) = delete;

  /// \brief Copies the data from some other array into this one.
  void CopyFrom(const nsArrayPtr<const T>& other, nsAllocator* pAllocator); // [tested]

  /// \brief Moves the data from some other array into this one.
  void MoveFrom(nsSmallArrayBase<T, Size>&& other, nsAllocator* pAllocator); // [tested]

  /// \brief Conversion to const nsArrayPtr.
  operator nsArrayPtr<const T>() const; // [tested]

  /// \brief Conversion to nsArrayPtr.
  operator nsArrayPtr<T>(); // [tested]

  /// \brief Compares this array to another contiguous array type.
  bool operator==(const nsSmallArrayBase<T, Size>& rhs) const; // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsSmallArrayBase<T, Size>&);

#if NS_DISABLED(NS_USE_CPP20_OPERATORS)
  bool operator==(const nsArrayPtr<const T>& rhs) const; // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsArrayPtr<const T>&);
#endif
  /// \brief Returns the element at the given index. Does bounds checks in debug builds.
  const T& operator[](nsUInt32 uiIndex) const; // [tested]

  /// \brief Returns the element at the given index. Does bounds checks in debug builds.
  T& operator[](nsUInt32 uiIndex); // [tested]

  /// \brief Resizes the array to have exactly uiCount elements. Default constructs extra elements if the array is grown.
  void SetCount(nsUInt16 uiCount, nsAllocator* pAllocator); // [tested]

  /// \brief Resizes the array to have exactly uiCount elements. Constructs all new elements by copying the FillValue.
  void SetCount(nsUInt16 uiCount, const T& fillValue, nsAllocator* pAllocator); // [tested]

  /// \brief Resizes the array to have exactly uiCount elements. Extra elements might be uninitialized.
  template <typename = void>                                             // Template is used to only conditionally compile this function in when it is actually used.
  void SetCountUninitialized(nsUInt16 uiCount, nsAllocator* pAllocator); // [tested]

  /// \brief Ensures the container has at least \a uiCount elements. Ie. calls SetCount() if the container has fewer elements, does nothing
  /// otherwise.
  void EnsureCount(nsUInt16 uiCount, nsAllocator* pAllocator); // [tested]

  /// \brief Returns the number of active elements in the array.
  nsUInt32 GetCount() const; // [tested]

  /// \brief Returns true, if the array does not contain any elements.
  bool IsEmpty() const; // [tested]

  /// \brief Clears the array.
  void Clear(); // [tested]

  /// \brief Checks whether the given value can be found in the array. O(n) complexity.
  bool Contains(const T& value) const; // [tested]

  /// \brief Inserts value at index by shifting all following elements.
  void Insert(const T& value, nsUInt32 uiIndex, nsAllocator* pAllocator); // [tested]

  /// \brief Inserts value at index by shifting all following elements.
  void Insert(T&& value, nsUInt32 uiIndex, nsAllocator* pAllocator); // [tested]

  /// \brief Removes the first occurrence of value and fills the gap by shifting all following elements
  bool RemoveAndCopy(const T& value); // [tested]

  /// \brief Removes the first occurrence of value and fills the gap by swapping in the last element
  bool RemoveAndSwap(const T& value); // [tested]

  /// \brief Removes the element at index and fills the gap by shifting all following elements
  void RemoveAtAndCopy(nsUInt32 uiIndex, nsUInt16 uiNumElements = 1); // [tested]

  /// \brief Removes the element at index and fills the gap by swapping in the last element
  void RemoveAtAndSwap(nsUInt32 uiIndex, nsUInt16 uiNumElements = 1); // [tested]

  /// \brief Searches for the first occurrence of the given value and returns its index or nsInvalidIndex if not found.
  nsUInt32 IndexOf(const T& value, nsUInt32 uiStartIndex = 0) const; // [tested]

  /// \brief Searches for the last occurrence of the given value and returns its index or nsInvalidIndex if not found.
  nsUInt32 LastIndexOf(const T& value, nsUInt32 uiStartIndex = nsSmallInvalidIndex) const; // [tested]

  /// \brief Grows the array by one element and returns a reference to the newly created element.
  T& ExpandAndGetRef(nsAllocator* pAllocator); // [tested]

  /// \brief Pushes value at the end of the array.
  void PushBack(const T& value, nsAllocator* pAllocator); // [tested]

  /// \brief Pushes value at the end of the array.
  void PushBack(T&& value, nsAllocator* pAllocator); // [tested]

  /// \brief Pushes value at the end of the array. Does NOT ensure capacity.
  void PushBackUnchecked(const T& value); // [tested]

  /// \brief Pushes value at the end of the array. Does NOT ensure capacity.
  void PushBackUnchecked(T&& value); // [tested]

  /// \brief Pushes all elements in range at the end of the array. Increases the capacity if necessary.
  void PushBackRange(const nsArrayPtr<const T>& range, nsAllocator* pAllocator); // [tested]

  /// \brief Removes count elements from the end of the array.
  void PopBack(nsUInt32 uiCountToRemove = 1); // [tested]

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
  nsArrayPtr<T> GetArrayPtr(); // [tested]

  /// \brief Returns an array pointer to the array data, or an empty array pointer if the array is empty.
  nsArrayPtr<const T> GetArrayPtr() const; // [tested]

  /// \brief Returns a byte array pointer to the array data, or an empty array pointer if the array is empty.
  nsArrayPtr<typename nsArrayPtr<T>::ByteType> GetByteArrayPtr(); // [tested]

  /// \brief Returns a byte array pointer to the array data, or an empty array pointer if the array is empty.
  nsArrayPtr<typename nsArrayPtr<const T>::ByteType> GetByteArrayPtr() const; // [tested]

  /// \brief Expands the array so it can at least store the given capacity.
  void Reserve(nsUInt16 uiCapacity, nsAllocator* pAllocator); // [tested]

  /// \brief Tries to compact the array to avoid wasting memory. The resulting capacity is at least 'GetCount' (no elements get removed). Will
  /// deallocate all data, if the array is empty.
  void Compact(nsAllocator* pAllocator); // [tested]

  /// \brief Returns the reserved number of elements that the array can hold without reallocating.
  nsUInt32 GetCapacity() const { return m_uiCapacity; }

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  nsUInt64 GetHeapMemoryUsage() const; // [tested]

  using const_iterator = const T*;
  using const_reverse_iterator = const_reverse_pointer_iterator<T>;
  using iterator = T*;
  using reverse_iterator = reverse_pointer_iterator<T>;

  template <typename U>
  const U& GetUserData() const; // [tested]

  template <typename U>
  U& GetUserData();             // [tested]

protected:
  enum
  {
    CAPACITY_ALIGNMENT = 4
  };

  void SetCapacity(nsUInt16 uiCapacity, nsAllocator* pAllocator);

  T* GetElementsPtr();
  const T* GetElementsPtr() const;

  nsUInt16 m_uiCount = 0;
  nsUInt16 m_uiCapacity = Size;

  nsUInt32 m_uiUserData = 0;

  union
  {
    struct alignas(NS_ALIGNMENT_OF(T))
    {
      nsUInt8 m_StaticData[Size * sizeof(T)];
    };

    T* m_pElements = nullptr;
  };
};

//////////////////////////////////////////////////////////////////////////

/// \brief \see nsSmallArrayBase
template <typename T, nsUInt16 Size, typename AllocatorWrapper = nsDefaultAllocatorWrapper>
class nsSmallArray : public nsSmallArrayBase<T, Size>
{
  using SUPER = nsSmallArrayBase<T, Size>;

public:
  // Only if the stored type is either POD or relocatable the hybrid array itself is also relocatable.
  NS_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T);

  nsSmallArray();

  nsSmallArray(const nsSmallArray<T, Size, AllocatorWrapper>& other);
  nsSmallArray(const nsArrayPtr<const T>& other);
  nsSmallArray(nsSmallArray<T, Size, AllocatorWrapper>&& other);

  ~nsSmallArray();

  void operator=(const nsSmallArray<T, Size, AllocatorWrapper>& rhs);
  void operator=(const nsArrayPtr<const T>& rhs);
  void operator=(nsSmallArray<T, Size, AllocatorWrapper>&& rhs) noexcept;

  void SetCount(nsUInt16 uiCount);                      // [tested]
  void SetCount(nsUInt16 uiCount, const T& fillValue);  // [tested]
  void EnsureCount(nsUInt16 uiCount);                   // [tested]

  template <typename = void>
  void SetCountUninitialized(nsUInt16 uiCount);         // [tested]

  void InsertAt(nsUInt32 uiIndex, const T& value);      // [tested]
  void InsertAt(nsUInt32 uiIndex, T&& value);           // [tested]

  T& ExpandAndGetRef();                                 // [tested]
  void PushBack(const T& value);                        // [tested]
  void PushBack(T&& value);                             // [tested]
  void PushBackRange(const nsArrayPtr<const T>& range); // [tested]

  void Reserve(nsUInt16 uiCapacity);
  void Compact();
};

#include <Foundation/Containers/Implementation/SmallArray_inl.h>
