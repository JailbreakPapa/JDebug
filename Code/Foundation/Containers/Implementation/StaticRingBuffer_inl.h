#pragma once

template <typename T, nsUInt32 C>
nsStaticRingBuffer<T, C>::nsStaticRingBuffer()
{
  m_pElements = GetStaticArray();
  m_uiFirstElement = 0;
  m_uiCount = 0;
}

template <typename T, nsUInt32 C>
nsStaticRingBuffer<T, C>::nsStaticRingBuffer(const nsStaticRingBuffer<T, C>& rhs)
{
  m_pElements = GetStaticArray();
  m_uiFirstElement = 0;
  m_uiCount = 0;

  *this = rhs;
}

template <typename T, nsUInt32 C>
nsStaticRingBuffer<T, C>::~nsStaticRingBuffer()
{
  Clear();
}

template <typename T, nsUInt32 C>
void nsStaticRingBuffer<T, C>::operator=(const nsStaticRingBuffer<T, C>& rhs)
{
  Clear();

  for (nsUInt32 i = 0; i < rhs.GetCount(); ++i)
    PushBack(rhs[i]);
}

template <typename T, nsUInt32 C>
bool nsStaticRingBuffer<T, C>::operator==(const nsStaticRingBuffer<T, C>& rhs) const
{
  if (GetCount() != rhs.GetCount())
    return false;

  for (nsUInt32 i = 0; i < m_uiCount; ++i)
  {
    if ((*this)[i] != rhs[i])
      return false;
  }

  return true;
}

template <typename T, nsUInt32 C>
void nsStaticRingBuffer<T, C>::PushBack(const T& element)
{
  NS_ASSERT_DEV(CanAppend(), "The ring-buffer is full, no elements can be appended before removing one.");

  const nsUInt32 uiLastElement = (m_uiFirstElement + m_uiCount) % C;

  nsMemoryUtils::CopyConstruct(&m_pElements[uiLastElement], element, 1);
  ++m_uiCount;
}

template <typename T, nsUInt32 C>
void nsStaticRingBuffer<T, C>::PushBack(T&& element)
{
  NS_ASSERT_DEV(CanAppend(), "The ring-buffer is full, no elements can be appended before removing one.");

  const nsUInt32 uiLastElement = (m_uiFirstElement + m_uiCount) % C;

  nsMemoryUtils::MoveConstruct(&m_pElements[uiLastElement], std::move(element));
  ++m_uiCount;
}

template <typename T, nsUInt32 C>
T& nsStaticRingBuffer<T, C>::PeekBack()
{
  NS_ASSERT_DEV(!IsEmpty(), "The ring-buffer is empty, cannot peek at the last element.");

  const nsUInt32 uiLastElement = (m_uiFirstElement + m_uiCount - 1) % C;
  return m_pElements[uiLastElement];
}

template <typename T, nsUInt32 C>
const T& nsStaticRingBuffer<T, C>::PeekBack() const
{
  NS_ASSERT_DEV(!IsEmpty(), "The ring-buffer is empty, cannot peek at the last element.");

  const nsUInt32 uiLastElement = (m_uiFirstElement + m_uiCount - 1) % C;
  return m_pElements[uiLastElement];
}

template <typename T, nsUInt32 C>
void nsStaticRingBuffer<T, C>::PopFront(nsUInt32 uiElements)
{
  NS_ASSERT_DEV(m_uiCount >= uiElements, "The ring-buffer contains {0} elements, cannot remove {1} elements from it.", m_uiCount, uiElements);

  while (uiElements > 0)
  {
    nsMemoryUtils::Destruct(&m_pElements[m_uiFirstElement], 1);
    ++m_uiFirstElement;
    m_uiFirstElement %= C;
    --m_uiCount;

    --uiElements;
  }
}

template <typename T, nsUInt32 C>
NS_FORCE_INLINE const T& nsStaticRingBuffer<T, C>::PeekFront() const
{
  NS_ASSERT_DEV(!IsEmpty(), "The ring-buffer is empty, cannot peek at the first element.");

  return m_pElements[m_uiFirstElement];
}

template <typename T, nsUInt32 C>
NS_FORCE_INLINE T& nsStaticRingBuffer<T, C>::PeekFront()
{
  NS_ASSERT_DEV(!IsEmpty(), "The ring-buffer is empty, cannot peek at the first element.");

  return m_pElements[m_uiFirstElement];
}

template <typename T, nsUInt32 C>
NS_FORCE_INLINE const T& nsStaticRingBuffer<T, C>::operator[](nsUInt32 uiIndex) const
{
  NS_ASSERT_DEBUG(uiIndex < m_uiCount, "The ring-buffer only has {0} elements, cannot access element {1}.", m_uiCount, uiIndex);

  return m_pElements[(m_uiFirstElement + uiIndex) % C];
}

template <typename T, nsUInt32 C>
NS_FORCE_INLINE T& nsStaticRingBuffer<T, C>::operator[](nsUInt32 uiIndex)
{
  NS_ASSERT_DEBUG(uiIndex < m_uiCount, "The ring-buffer only has {0} elements, cannot access element {1}.", m_uiCount, uiIndex);

  return m_pElements[(m_uiFirstElement + uiIndex) % C];
}

template <typename T, nsUInt32 C>
NS_ALWAYS_INLINE nsUInt32 nsStaticRingBuffer<T, C>::GetCount() const
{
  return m_uiCount;
}

template <typename T, nsUInt32 C>
NS_ALWAYS_INLINE bool nsStaticRingBuffer<T, C>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename T, nsUInt32 C>
NS_ALWAYS_INLINE bool nsStaticRingBuffer<T, C>::CanAppend(nsUInt32 uiElements)
{
  return (m_uiCount + uiElements) <= C;
}

template <typename T, nsUInt32 C>
void nsStaticRingBuffer<T, C>::Clear()
{
  while (!IsEmpty())
    PopFront();
}

template <typename T, nsUInt32 C>
NS_ALWAYS_INLINE T* nsStaticRingBuffer<T, C>::GetStaticArray()
{
  return reinterpret_cast<T*>(m_Data);
}
