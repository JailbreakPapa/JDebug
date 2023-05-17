#pragma once

#include <Foundation/Containers/DynamicArray.h>

/// \brief A hybrid array uses in-place storage to handle the first few elements without any allocation. It dynamically resizes when more elements are needed.
///
/// It is often more efficient to use a hybrid array, rather than a dynamic array, when the number of needed elements is typically low or when the array is used only temporarily. In this case costly allocations can often be prevented entirely.
/// However, if the number of elements is unpredictable or usually very large, prefer a dynamic array, to avoid wasting (stack) memory for a hybrid array that is rarely large enough to be used.
/// The wdHybridArray is derived from wdDynamicArray and can therefore be passed to functions that expect an wdDynamicArray, even for output.
template <typename T, wdUInt32 Size, typename AllocatorWrapper = wdDefaultAllocatorWrapper>
class wdHybridArray : public wdDynamicArray<T, AllocatorWrapper>
{
public:
  // Only if the stored type is either POD or relocatable the hybrid array itself is also relocatable.
  WD_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T);

  /// \brief Creates an empty array. Does not allocate any data yet.
  wdHybridArray(); // [tested]

  /// \brief Creates an empty array. Does not allocate any data yet.
  explicit wdHybridArray(wdAllocatorBase* pAllocator); // [tested]

  /// \brief Creates a copy of the given array.
  wdHybridArray(const wdHybridArray<T, Size, AllocatorWrapper>& other); // [tested]

  /// \brief Creates a copy of the given array.
  explicit wdHybridArray(const wdArrayPtr<const T>& other); // [tested]

  /// \brief Moves the given array.
  wdHybridArray(wdHybridArray<T, Size, AllocatorWrapper>&& other); // [tested]

  /// \brief Copies the data from some other contiguous array into this one.
  void operator=(const wdHybridArray<T, Size, AllocatorWrapper>& rhs); // [tested]

  /// \brief Copies the data from some other contiguous array into this one.
  void operator=(const wdArrayPtr<const T>& rhs); // [tested]

  /// \brief Moves the data from some other contiguous array into this one.
  void operator=(wdHybridArray<T, Size, AllocatorWrapper>&& rhs) noexcept; // [tested]

protected:
  /// \brief The fixed size array.
  struct alignas(WD_ALIGNMENT_OF(T))
  {
    wdUInt8 m_StaticData[Size * sizeof(T)];
  };

  WD_ALWAYS_INLINE T* GetStaticArray() { return reinterpret_cast<T*>(m_StaticData); }

  WD_ALWAYS_INLINE const T* GetStaticArray() const { return reinterpret_cast<const T*>(m_StaticData); }
};

#include <Foundation/Containers/Implementation/HybridArray_inl.h>
