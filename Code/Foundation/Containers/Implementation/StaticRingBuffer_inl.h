#pragma once

template <typename T, wdUInt32 C>
wdStaticRingBuffer<T, C>::wdStaticRingBuffer()
{
  m_pElements = GetStaticArray();
  m_uiFirstElement = 0;
  m_uiCount = 0;
}

template <typename T, wdUInt32 C>
wdStaticRingBuffer<T, C>::wdStaticRingBuffer(const wdStaticRingBuffer<T, C>& rhs)
{
  m_pElements = GetStaticArray();
  m_uiFirstElement = 0;
  m_uiCount = 0;

  *this = rhs;
}

template <typename T, wdUInt32 C>
wdStaticRingBuffer<T, C>::~wdStaticRingBuffer()
{
  Clear();
}

template <typename T, wdUInt32 C>
void wdStaticRingBuffer<T, C>::operator=(const wdStaticRingBuffer<T, C>& rhs)
{
  Clear();

  for (wdUInt32 i = 0; i < rhs.GetCount(); ++i)
    PushBack(rhs[i]);
}

template <typename T, wdUInt32 C>
bool wdStaticRingBuffer<T, C>::operator==(const wdStaticRingBuffer<T, C>& rhs) const
{
  if (GetCount() != rhs.GetCount())
    return false;

  for (wdUInt32 i = 0; i < m_uiCount; ++i)
  {
    if ((*this)[i] != rhs[i])
      return false;
  }

  return true;
}

template <typename T, wdUInt32 C>
WD_ALWAYS_INLINE bool wdStaticRingBuffer<T, C>::operator!=(const wdStaticRingBuffer<T, C>& rhs) const
{
  return !(*this == rhs);
}

template <typename T, wdUInt32 C>
void wdStaticRingBuffer<T, C>::PushBack(const T& element)
{
  WD_ASSERT_DEV(CanAppend(), "The ring-buffer is full, no elements can be appended before removing one.");

  const wdUInt32 uiLastElement = (m_uiFirstElement + m_uiCount) % C;

  wdMemoryUtils::CopyConstruct(&m_pElements[uiLastElement], element, 1);
  ++m_uiCount;
}

template <typename T, wdUInt32 C>
void wdStaticRingBuffer<T, C>::PushBack(T&& element)
{
  WD_ASSERT_DEV(CanAppend(), "The ring-buffer is full, no elements can be appended before removing one.");

  const wdUInt32 uiLastElement = (m_uiFirstElement + m_uiCount) % C;

  wdMemoryUtils::MoveConstruct(&m_pElements[uiLastElement], std::move(element));
  ++m_uiCount;
}

template <typename T, wdUInt32 C>
T& wdStaticRingBuffer<T, C>::PeekBack()
{
  WD_ASSERT_DEV(!IsEmpty(), "The ring-buffer is empty, cannot peek at the last element.");

  const wdUInt32 uiLastElement = (m_uiFirstElement + m_uiCount - 1) % C;
  return m_pElements[uiLastElement];
}

template <typename T, wdUInt32 C>
const T& wdStaticRingBuffer<T, C>::PeekBack() const
{
  WD_ASSERT_DEV(!IsEmpty(), "The ring-buffer is empty, cannot peek at the last element.");

  const wdUInt32 uiLastElement = (m_uiFirstElement + m_uiCount - 1) % C;
  return m_pElements[uiLastElement];
}

template <typename T, wdUInt32 C>
void wdStaticRingBuffer<T, C>::PopFront(wdUInt32 uiElements)
{
  WD_ASSERT_DEV(m_uiCount >= uiElements, "The ring-buffer contains {0} elements, cannot remove {1} elements from it.", m_uiCount, uiElements);

  while (uiElements > 0)
  {
    wdMemoryUtils::Destruct(&m_pElements[m_uiFirstElement], 1);
    ++m_uiFirstElement;
    m_uiFirstElement %= C;
    --m_uiCount;

    --uiElements;
  }
}

template <typename T, wdUInt32 C>
WD_FORCE_INLINE const T& wdStaticRingBuffer<T, C>::PeekFront() const
{
  WD_ASSERT_DEV(!IsEmpty(), "The ring-buffer is empty, cannot peek at the first element.");

  return m_pElements[m_uiFirstElement];
}

template <typename T, wdUInt32 C>
WD_FORCE_INLINE T& wdStaticRingBuffer<T, C>::PeekFront()
{
  WD_ASSERT_DEV(!IsEmpty(), "The ring-buffer is empty, cannot peek at the first element.");

  return m_pElements[m_uiFirstElement];
}

template <typename T, wdUInt32 C>
WD_FORCE_INLINE const T& wdStaticRingBuffer<T, C>::operator[](wdUInt32 uiIndex) const
{
  WD_ASSERT_DEV(uiIndex < m_uiCount, "The ring-buffer only has {0} elements, cannot access element {1}.", m_uiCount, uiIndex);

  return m_pElements[(m_uiFirstElement + uiIndex) % C];
}

template <typename T, wdUInt32 C>
WD_FORCE_INLINE T& wdStaticRingBuffer<T, C>::operator[](wdUInt32 uiIndex)
{
  WD_ASSERT_DEV(uiIndex < m_uiCount, "The ring-buffer only has {0} elements, cannot access element {1}.", m_uiCount, uiIndex);

  return m_pElements[(m_uiFirstElement + uiIndex) % C];
}

template <typename T, wdUInt32 C>
WD_ALWAYS_INLINE wdUInt32 wdStaticRingBuffer<T, C>::GetCount() const
{
  return m_uiCount;
}

template <typename T, wdUInt32 C>
WD_ALWAYS_INLINE bool wdStaticRingBuffer<T, C>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename T, wdUInt32 C>
WD_ALWAYS_INLINE bool wdStaticRingBuffer<T, C>::CanAppend(wdUInt32 uiElements)
{
  return (m_uiCount + uiElements) <= C;
}

template <typename T, wdUInt32 C>
void wdStaticRingBuffer<T, C>::Clear()
{
  while (!IsEmpty())
    PopFront();
}

template <typename T, wdUInt32 C>
WD_ALWAYS_INLINE T* wdStaticRingBuffer<T, C>::GetStaticArray()
{
  return reinterpret_cast<T*>(m_Data);
}
