#pragma once

#include <Foundation/Containers/StaticArray.h>

/// \brief A ring-buffer container that will use a static array of a given capacity to cycle through elements.
///
/// If you need a dynamic ring-buffer, use an nsDeque.
template <typename T, nsUInt32 Capacity>
class nsStaticRingBuffer
{
public:
  NS_CHECK_AT_COMPILETIME_MSG(Capacity > 1, "ORLY?");

  /// \brief Constructs an empty ring-buffer.
  nsStaticRingBuffer(); // [tested]

  /// \brief Copies the content from rhs into this ring-buffer.
  nsStaticRingBuffer(const nsStaticRingBuffer<T, Capacity>& rhs); // [tested]

  /// \brief Destructs all remaining elements.
  ~nsStaticRingBuffer(); // [tested]

  /// \brief Copies the content from rhs into this ring-buffer.
  void operator=(const nsStaticRingBuffer<T, Capacity>& rhs); // [tested]

  /// \brief Compares two ring-buffers for equality.
  bool operator==(const nsStaticRingBuffer<T, Capacity>& rhs) const; // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsStaticRingBuffer<T, Capacity>&);

  /// \brief Appends an element at the end of the ring-buffer. Asserts that CanAppend() is true.
  void PushBack(const T& element); // [tested]

  /// \brief Appends an element at the end of the ring-buffer. Asserts that CanAppend() is true.
  void PushBack(T&& element); // [tested]

  /// \brief Accesses the latest element in the ring-buffer.
  T& PeekBack(); // [tested]

  /// \brief Accesses the latest element in the ring-buffer.
  const T& PeekBack() const; // [tested]

  /// \brief Removes the oldest element from the ring-buffer.
  void PopFront(nsUInt32 uiElements = 1); // [tested]

  /// \brief Accesses the oldest element in the ring-buffer.
  const T& PeekFront() const; // [tested]

  /// \brief Accesses the oldest element in the ring-buffer.
  T& PeekFront(); // [tested]

  /// \brief Accesses the n-th element in the ring-buffer.
  const T& operator[](nsUInt32 uiIndex) const; // [tested]

  /// \brief Accesses the n-th element in the ring-buffer.
  T& operator[](nsUInt32 uiIndex); // [tested]

  /// \brief Returns the number of elements that are currently in the ring-buffer.
  nsUInt32 GetCount() const; // [tested]

  /// \brief Returns true if the ring-buffer currently contains no elements.
  bool IsEmpty() const; // [tested]

  /// \brief Returns true, if the ring-buffer can store at least uiElements additional elements.
  bool CanAppend(nsUInt32 uiElements = 1); // [tested]

  /// \brief Destructs all elements in the ring-buffer.
  void Clear(); // [tested]

private:
  T* GetStaticArray();

  /// \brief The fixed size array.
  struct alignas(NS_ALIGNMENT_OF(T))
  {
    nsUInt8 m_Data[Capacity * sizeof(T)];
  };

  T* m_pElements;
  nsUInt32 m_uiCount;
  nsUInt32 m_uiFirstElement;
};

#include <Foundation/Containers/Implementation/StaticRingBuffer_inl.h>
