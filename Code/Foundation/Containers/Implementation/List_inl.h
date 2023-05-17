#pragma once

#include <Foundation/Math/Math.h>

// **** ListElement ****

template <typename T>
wdListBase<T>::ListElementBase::ListElementBase()
  : m_pPrev(nullptr)
  , m_pNext(nullptr)
{
}

template <typename T>
wdListBase<T>::ListElement::ListElement(const T& data)
  : m_Data(data)
{
}

// **** wdListBase ****

template <typename T>
wdListBase<T>::wdListBase(wdAllocatorBase* pAllocator)
  : m_End(reinterpret_cast<ListElement*>(&m_Last))
  , m_uiCount(0)
  , m_Elements(pAllocator)
  , m_pFreeElementStack(nullptr)
{
  m_First.m_pNext = reinterpret_cast<ListElement*>(&m_Last);
  m_Last.m_pPrev = reinterpret_cast<ListElement*>(&m_First);
}

template <typename T>
wdListBase<T>::wdListBase(const wdListBase<T>& cc, wdAllocatorBase* pAllocator)
  : m_End(reinterpret_cast<ListElement*>(&m_Last))
  , m_uiCount(0)
  , m_Elements(pAllocator)
  , m_pFreeElementStack(nullptr)
{
  m_First.m_pNext = reinterpret_cast<ListElement*>(&m_Last);
  m_Last.m_pPrev = reinterpret_cast<ListElement*>(&m_First);

  operator=(cc);
}

template <typename T>
wdListBase<T>::~wdListBase()
{
  Clear();
}

template <typename T>
void wdListBase<T>::operator=(const wdListBase<T>& cc)
{
  Clear();
  Insert(GetIterator(), cc.GetIterator(), cc.GetEndIterator());
}

template <typename T>
typename wdListBase<T>::ListElement* wdListBase<T>::AcquireNode(const T& data)
{
  ListElement* pNode;

  if (m_pFreeElementStack == nullptr)
  {
    m_Elements.PushBack();
    pNode = &m_Elements.PeekBack();
  }
  else
  {
    pNode = m_pFreeElementStack;
    m_pFreeElementStack = m_pFreeElementStack->m_pNext;
  }

  wdMemoryUtils::Construct<ListElement>(pNode, 1);
  pNode->m_Data = data;
  return pNode;
}

template <typename T>
void wdListBase<T>::ReleaseNode(ListElement* pNode)
{
  wdMemoryUtils::Destruct<ListElement>(pNode, 1);

  if (pNode == &m_Elements.PeekBack())
  {
    m_Elements.PopBack();
  }
  else if (pNode == &m_Elements.PeekFront())
  {
    m_Elements.PopFront();
  }
  else
  {
    pNode->m_pNext = m_pFreeElementStack;
    m_pFreeElementStack = pNode;
  }

  --m_uiCount;
}


template <typename T>
WD_ALWAYS_INLINE typename wdListBase<T>::Iterator wdListBase<T>::GetIterator()
{
  return Iterator(m_First.m_pNext);
}

template <typename T>
WD_ALWAYS_INLINE typename wdListBase<T>::Iterator wdListBase<T>::GetLastIterator()
{
  return Iterator(m_Last.m_pPrev);
}

template <typename T>
WD_ALWAYS_INLINE typename wdListBase<T>::Iterator wdListBase<T>::GetEndIterator()
{
  return m_End;
}

template <typename T>
WD_ALWAYS_INLINE typename wdListBase<T>::ConstIterator wdListBase<T>::GetIterator() const
{
  return ConstIterator(m_First.m_pNext);
}

template <typename T>
WD_ALWAYS_INLINE typename wdListBase<T>::ConstIterator wdListBase<T>::GetLastIterator() const
{
  return ConstIterator(m_Last.m_pPrev);
}

template <typename T>
WD_ALWAYS_INLINE typename wdListBase<T>::ConstIterator wdListBase<T>::GetEndIterator() const
{
  return m_End;
}

template <typename T>
WD_ALWAYS_INLINE wdUInt32 wdListBase<T>::GetCount() const
{
  return m_uiCount;
}

template <typename T>
WD_ALWAYS_INLINE bool wdListBase<T>::IsEmpty() const
{
  return (m_uiCount == 0);
}

template <typename T>
void wdListBase<T>::Clear()
{
  if (!IsEmpty())
    Remove(GetIterator(), GetEndIterator());

  m_pFreeElementStack = nullptr;
  m_Elements.Clear();
}

template <typename T>
WD_FORCE_INLINE void wdListBase<T>::Compact()
{
  m_Elements.Compact();
}

template <typename T>
WD_FORCE_INLINE T& wdListBase<T>::PeekFront()
{
  WD_ASSERT_DEV(!IsEmpty(), "The container is empty.");

  return m_First.m_pNext->m_Data;
}

template <typename T>
WD_FORCE_INLINE T& wdListBase<T>::PeekBack()
{
  WD_ASSERT_DEV(!IsEmpty(), "The container is empty.");

  return m_Last.m_pPrev->m_Data;
}

template <typename T>
WD_FORCE_INLINE const T& wdListBase<T>::PeekFront() const
{
  WD_ASSERT_DEV(!IsEmpty(), "The container is empty.");

  return m_First.m_pNext->m_Data;
}

template <typename T>
WD_FORCE_INLINE const T& wdListBase<T>::PeekBack() const
{
  WD_ASSERT_DEV(!IsEmpty(), "The container is empty.");

  return m_Last.m_pPrev->m_Data;
}


template <typename T>
WD_ALWAYS_INLINE void wdListBase<T>::PushBack()
{
  PushBack(T());
}

template <typename T>
WD_ALWAYS_INLINE void wdListBase<T>::PushBack(const T& element)
{
  Insert(GetEndIterator(), element);
}

template <typename T>
WD_ALWAYS_INLINE void wdListBase<T>::PushFront()
{
  PushFront(T());
}

template <typename T>
WD_ALWAYS_INLINE void wdListBase<T>::PushFront(const T& element)
{
  Insert(GetIterator(), element);
}

template <typename T>
WD_FORCE_INLINE void wdListBase<T>::PopBack()
{
  WD_ASSERT_DEV(!IsEmpty(), "The container is empty.");

  Remove(Iterator(m_Last.m_pPrev));
}

template <typename T>
void wdListBase<T>::PopFront()
{
  WD_ASSERT_DEV(!IsEmpty(), "The container is empty.");

  Remove(Iterator(m_First.m_pNext));
}

template <typename T>
typename wdListBase<T>::Iterator wdListBase<T>::Insert(const Iterator& pos, const T& data)
{
  WD_ASSERT_DEV(pos.m_pElement != nullptr, "The iterator (pos) is invalid.");

  ++m_uiCount;
  ListElement* elem = AcquireNode(data);

  elem->m_pNext = pos.m_pElement;
  elem->m_pPrev = pos.m_pElement->m_pPrev;

  pos.m_pElement->m_pPrev->m_pNext = elem;
  pos.m_pElement->m_pPrev = elem;

  return Iterator(elem);
}

template <typename T>
void wdListBase<T>::Insert(const Iterator& pos, ConstIterator first, const ConstIterator& last)
{
  WD_ASSERT_DEV(pos.m_pElement != nullptr, "The iterator (pos) is invalid.");
  WD_ASSERT_DEV(first.m_pElement != nullptr, "The iterator (first) is invalid.");
  WD_ASSERT_DEV(last.m_pElement != nullptr, "The iterator (last) is invalid.");

  while (first != last)
  {
    Insert(pos, *first);
    ++first;
  }
}

template <typename T>
typename wdListBase<T>::Iterator wdListBase<T>::Remove(const Iterator& pos)
{
  WD_ASSERT_DEV(!IsEmpty(), "The container is empty.");
  WD_ASSERT_DEV(pos.m_pElement != nullptr, "The iterator (pos) is invalid.");

  ListElement* pPrev = pos.m_pElement->m_pPrev;
  ListElement* pNext = pos.m_pElement->m_pNext;

  pPrev->m_pNext = pNext;
  pNext->m_pPrev = pPrev;

  ReleaseNode(pos.m_pElement);

  return Iterator(pNext);
}

template <typename T>
typename wdListBase<T>::Iterator wdListBase<T>::Remove(Iterator first, const Iterator& last)
{
  WD_ASSERT_DEV(!IsEmpty(), "The container is empty.");
  WD_ASSERT_DEV(first.m_pElement != nullptr, "The iterator (first) is invalid.");
  WD_ASSERT_DEV(last.m_pElement != nullptr, "The iterator (last) is invalid.");

  while (first != last)
    first = Remove(first);

  return last;
}

/*! If uiNewSize is smaller than the size of the list, elements are popped from the back, until the desired size is reached.
    If uiNewSize is larger than the size of the list, default-constructed elements are appended to the list, until the desired size is reached.
*/
template <typename T>
void wdListBase<T>::SetCount(wdUInt32 uiNewSize)
{
  while (m_uiCount > uiNewSize)
    PopBack();

  while (m_uiCount < uiNewSize)
    PushBack();
}

template <typename T>
bool wdListBase<T>::operator==(const wdListBase<T>& rhs) const
{
  if (GetCount() != rhs.GetCount())
    return false;

  auto itLhs = GetIterator();
  auto itRhs = rhs.GetIterator();

  while (itLhs.IsValid())
  {
    if (*itLhs != *itRhs)
      return false;

    ++itLhs;
    ++itRhs;
  }

  return true;
}

template <typename T>
bool wdListBase<T>::operator!=(const wdListBase<T>& rhs) const
{
  return !operator==(rhs);
}

template <typename T, typename A>
wdList<T, A>::wdList()
  : wdListBase<T>(A::GetAllocator())
{
}

template <typename T, typename A>
wdList<T, A>::wdList(wdAllocatorBase* pAllocator)
  : wdListBase<T>(pAllocator)
{
}

template <typename T, typename A>
wdList<T, A>::wdList(const wdList<T, A>& other)
  : wdListBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
wdList<T, A>::wdList(const wdListBase<T>& other)
  : wdListBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
void wdList<T, A>::operator=(const wdList<T, A>& rhs)
{
  wdListBase<T>::operator=(rhs);
}

template <typename T, typename A>
void wdList<T, A>::operator=(const wdListBase<T>& rhs)
{
  wdListBase<T>::operator=(rhs);
}
