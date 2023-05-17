#pragma once

#include <Foundation/Containers/ArrayBase.h>

/// \brief Wraps a C-style array, which has a fixed size at compile-time, with a more convenient interface.
///
/// wdStaticArray can be used to create a fixed size array, either on the stack or as a class member.
/// Additionally it allows to use that array as a 'cache', i.e. not all its elements need to be constructed.
/// As such it can be used whenever a fixed size array is sufficient, but a more powerful interface is desired,
/// and when the number of elements in an array is dynamic at run-time, but always capped at a fixed limit.
template <typename T, wdUInt32 Capacity>
class wdStaticArray : public wdArrayBase<T, wdStaticArray<T, Capacity>>
{
public:
  // Only if the stored type is either POD or relocatable the hybrid array itself is also relocatable.
  WD_DECLARE_MEM_RELOCATABLE_TYPE_CONDITIONAL(T);

  /// \brief Creates an empty array.
  wdStaticArray(); // [tested]

  /// \brief Creates a copy of the given array.
  wdStaticArray(const wdStaticArray<T, Capacity>& rhs); // [tested]

  /// \brief Creates a copy of the given array.
  template <wdUInt32 OtherCapacity>
  wdStaticArray(const wdStaticArray<T, OtherCapacity>& rhs); // [tested]

  /// \brief Creates a copy of the given array.
  explicit wdStaticArray(const wdArrayPtr<const T>& rhs); // [tested]

  /// \brief Destroys all objects.
  ~wdStaticArray(); // [tested]

  /// \brief Copies the data from some other contiguous array into this one.
  void operator=(const wdStaticArray<T, Capacity>& rhs); // [tested]

  /// \brief Copies the data from some other contiguous array into this one.
  template <wdUInt32 OtherCapacity>
  void operator=(const wdStaticArray<T, OtherCapacity>& rhs); // [tested]

  /// \brief Copies the data from some other contiguous array into this one.
  void operator=(const wdArrayPtr<const T>& rhs); // [tested]

  /// \brief For the static array Reserve is a no-op. However the function checks if the requested capacity is below or equal to the static capacity.
  void Reserve(wdUInt32 uiCapacity);

protected:
  T* GetElementsPtr();
  const T* GetElementsPtr() const;
  friend class wdArrayBase<T, wdStaticArray<T, Capacity>>;

private:
  T* GetStaticArray();
  const T* GetStaticArray() const;

  /// \brief The fixed size array.
  struct alignas(WD_ALIGNMENT_OF(T))
  {
    wdUInt8 m_Data[Capacity * sizeof(T)];
  };

  friend class wdArrayBase<T, wdStaticArray<T, Capacity>>;
};

// TODO WD_CHECK_AT_COMPILETIME_MSG with a ',' in the expression does not work
// WD_CHECK_AT_COMPILETIME_MSG(wdGetTypeClass< wdStaticArray<int, 4> >::value == 2, "static array is not memory relocatable");

#include <Foundation/Containers/Implementation/StaticArray_inl.h>
