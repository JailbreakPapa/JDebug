#pragma once

#include <Foundation/Containers/StaticArray.h>

/// \brief A ring-buffer container that will use a static array of a given capacity to cycle through elements.
///
/// If you need a dynamic ring-buffer, use an wdDeque.
template <typename T, wdUInt32 Capacity>
class wdStaticRingBuffer
{
public:
  WD_CHECK_AT_COMPILETIME_MSG(Capacity > 1, "ORLY?");

  /// \brief Constructs an empty ring-buffer.
  wdStaticRingBuffer(); // [tested]

  /// \brief Copies the content from rhs into this ring-buffer.
  wdStaticRingBuffer(const wdStaticRingBuffer<T, Capacity>& rhs); // [tested]

  /// \brief Destructs all remaining elements.
  ~wdStaticRingBuffer(); // [tested]

  /// \brief Copies the content from rhs into this ring-buffer.
  void operator=(const wdStaticRingBuffer<T, Capacity>& rhs); // [tested]

  /// \brief Compares two ring-buffers for equality.
  bool operator==(const wdStaticRingBuffer<T, Capacity>& rhs) const; // [tested]

  /// \brief Compares two ring-buffers for inequality.
  bool operator!=(const wdStaticRingBuffer<T, Capacity>& rhs) const; // [tested]

  /// \brief Appends an element at the end of the ring-buffer. Asserts that CanAppend() is true.
  void PushBack(const T& element); // [tested]

  /// \brief Appends an element at the end of the ring-buffer. Asserts that CanAppend() is true.
  void PushBack(T&& element); // [tested]

  /// \brief Accesses the latest element in the ring-buffer.
  T& PeekBack(); // [tested]

  /// \brief Accesses the latest element in the ring-buffer.
  const T& PeekBack() const; // [tested]

  /// \brief Removes the oldest element from the ring-buffer.
  void PopFront(wdUInt32 uiElements = 1); // [tested]

  /// \brief Accesses the oldest element in the ring-buffer.
  const T& PeekFront() const; // [tested]

  /// \brief Accesses the oldest element in the ring-buffer.
  T& PeekFront(); // [tested]

  /// \brief Accesses the n-th element in the ring-buffer.
  const T& operator[](wdUInt32 uiIndex) const; // [tested]

  /// \brief Accesses the n-th element in the ring-buffer.
  T& operator[](wdUInt32 uiIndex); // [tested]

  /// \brief Returns the number of elements that are currently in the ring-buffer.
  wdUInt32 GetCount() const; // [tested]

  /// \brief Returns true if the ring-buffer currently contains no elements.
  bool IsEmpty() const; // [tested]

  /// \brief Returns true, if the ring-buffer can store at least uiElements additional elements.
  bool CanAppend(wdUInt32 uiElements = 1); // [tested]

  /// \brief Destructs all elements in the ring-buffer.
  void Clear(); // [tested]

private:
  T* GetStaticArray();

  /// \brief The fixed size array.
  struct alignas(WD_ALIGNMENT_OF(T))
  {
    wdUInt8 m_Data[Capacity * sizeof(T)];
  };

  T* m_pElements;
  wdUInt32 m_uiCount;
  wdUInt32 m_uiFirstElement;
};

#include <Foundation/Containers/Implementation/StaticRingBuffer_inl.h>
