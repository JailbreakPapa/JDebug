#pragma once

#include <Foundation/Math/Math.h>

// **** ListElement ****

template <typename T>
nsListBase<T>::ListElementBase::ListElementBase()
  : m_pPrev(nullptr)
  , m_pNext(nullptr)
{
}

template <typename T>
nsListBase<T>::ListElement::ListElement(const T& data)
  : m_Data(data)
{
}

// **** nsListBase ****

template <typename T>
nsListBase<T>::nsListBase(nsAllocator* pAllocator)
  : m_End(reinterpret_cast<ListElement*>(&m_Last))
  , m_uiCount(0)
  , m_Elements(pAllocator)
  , m_pFreeElementStack(nullptr)
{
  m_First.m_pNext = reinterpret_cast<ListElement*>(&m_Last);
  m_Last.m_pPrev = reinterpret_cast<ListElement*>(&m_First);
}

template <typename T>
nsListBase<T>::nsListBase(const nsListBase<T>& cc, nsAllocator* pAllocator)
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
nsListBase<T>::~nsListBase()
{
  Clear();
}

template <typename T>
void nsListBase<T>::operator=(const nsListBase<T>& cc)
{
  Clear();
  Insert(GetIterator(), cc.GetIterator(), cc.GetEndIterator());
}

template <typename T>
typename nsListBase<T>::ListElement* nsListBase<T>::AcquireNode()
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

  nsMemoryUtils::Construct<SkipTrivialTypes, ListElement>(pNode, 1);
  return pNode;
}

template <typename T>
void nsListBase<T>::ReleaseNode(ListElement* pNode)
{
  nsMemoryUtils::Destruct<ListElement>(pNode, 1);

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
NS_ALWAYS_INLINE typename nsListBase<T>::Iterator nsListBase<T>::GetIterator()
{
  return Iterator(m_First.m_pNext);
}

template <typename T>
NS_ALWAYS_INLINE typename nsListBase<T>::Iterator nsListBase<T>::GetEndIterator()
{
  return m_End;
}

template <typename T>
NS_ALWAYS_INLINE typename nsListBase<T>::ConstIterator nsListBase<T>::GetIterator() const
{
  return ConstIterator(m_First.m_pNext);
}

template <typename T>
NS_ALWAYS_INLINE typename nsListBase<T>::ConstIterator nsListBase<T>::GetEndIterator() const
{
  return m_End;
}

template <typename T>
NS_ALWAYS_INLINE nsUInt32 nsListBase<T>::GetCount() const
{
  return m_uiCount;
}

template <typename T>
NS_ALWAYS_INLINE bool nsListBase<T>::IsEmpty() const
{
  return (m_uiCount == 0);
}

template <typename T>
void nsListBase<T>::Clear()
{
  if (!IsEmpty())
    Remove(GetIterator(), GetEndIterator());

  m_pFreeElementStack = nullptr;
  m_Elements.Clear();
}

template <typename T>
NS_FORCE_INLINE void nsListBase<T>::Compact()
{
  m_Elements.Compact();
}

template <typename T>
NS_FORCE_INLINE T& nsListBase<T>::PeekFront()
{
  NS_ASSERT_DEBUG(!IsEmpty(), "The container is empty.");

  return m_First.m_pNext->m_Data;
}

template <typename T>
NS_FORCE_INLINE T& nsListBase<T>::PeekBack()
{
  NS_ASSERT_DEBUG(!IsEmpty(), "The container is empty.");

  return m_Last.m_pPrev->m_Data;
}

template <typename T>
NS_FORCE_INLINE const T& nsListBase<T>::PeekFront() const
{
  NS_ASSERT_DEBUG(!IsEmpty(), "The container is empty.");

  return m_First.m_pNext->m_Data;
}

template <typename T>
NS_FORCE_INLINE const T& nsListBase<T>::PeekBack() const
{
  NS_ASSERT_DEBUG(!IsEmpty(), "The container is empty.");

  return m_Last.m_pPrev->m_Data;
}


template <typename T>
NS_ALWAYS_INLINE T& nsListBase<T>::PushBack()
{
  return *Insert(GetEndIterator());
}

template <typename T>
NS_ALWAYS_INLINE void nsListBase<T>::PushBack(const T& element)
{
  Insert(GetEndIterator(), element);
}

template <typename T>
NS_ALWAYS_INLINE T& nsListBase<T>::PushFront()
{
  return *Insert(GetIterator());
}

template <typename T>
NS_ALWAYS_INLINE void nsListBase<T>::PushFront(const T& element)
{
  Insert(GetIterator(), element);
}

template <typename T>
NS_FORCE_INLINE void nsListBase<T>::PopBack()
{
  NS_ASSERT_DEBUG(!IsEmpty(), "The container is empty.");

  Remove(Iterator(m_Last.m_pPrev));
}

template <typename T>
void nsListBase<T>::PopFront()
{
  NS_ASSERT_DEBUG(!IsEmpty(), "The container is empty.");

  Remove(Iterator(m_First.m_pNext));
}

template <typename T>
typename nsListBase<T>::Iterator nsListBase<T>::Insert(const Iterator& pos)
{
  NS_ASSERT_DEV(pos.m_pElement != nullptr, "The iterator (pos) is invalid.");

  ++m_uiCount;
  ListElement* elem = AcquireNode();

  elem->m_pNext = pos.m_pElement;
  elem->m_pPrev = pos.m_pElement->m_pPrev;

  pos.m_pElement->m_pPrev->m_pNext = elem;
  pos.m_pElement->m_pPrev = elem;

  return Iterator(elem);
}

template <typename T>
typename nsListBase<T>::Iterator nsListBase<T>::Insert(const Iterator& pos, const T& data)
{
  NS_ASSERT_DEV(pos.m_pElement != nullptr, "The iterator (pos) is invalid.");

  ++m_uiCount;
  ListElement* elem = AcquireNode();
  elem->m_Data = data;

  elem->m_pNext = pos.m_pElement;
  elem->m_pPrev = pos.m_pElement->m_pPrev;

  pos.m_pElement->m_pPrev->m_pNext = elem;
  pos.m_pElement->m_pPrev = elem;

  return Iterator(elem);
}

template <typename T>
void nsListBase<T>::Insert(const Iterator& pos, ConstIterator first, const ConstIterator& last)
{
  NS_ASSERT_DEV(pos.m_pElement != nullptr && first.m_pElement != nullptr && last.m_pElement != nullptr, "One of the iterators is invalid.");

  while (first != last)
  {
    Insert(pos, *first);
    ++first;
  }
}

template <typename T>
typename nsListBase<T>::Iterator nsListBase<T>::Remove(const Iterator& pos)
{
  NS_ASSERT_DEV(!IsEmpty(), "The container is empty.");
  NS_ASSERT_DEV(pos.m_pElement != nullptr, "The iterator (pos) is invalid.");

  ListElement* pPrev = pos.m_pElement->m_pPrev;
  ListElement* pNext = pos.m_pElement->m_pNext;

  pPrev->m_pNext = pNext;
  pNext->m_pPrev = pPrev;

  ReleaseNode(pos.m_pElement);

  return Iterator(pNext);
}

template <typename T>
typename nsListBase<T>::Iterator nsListBase<T>::Remove(Iterator first, const Iterator& last)
{
  NS_ASSERT_DEV(!IsEmpty(), "The container is empty.");
  NS_ASSERT_DEV(first.m_pElement != nullptr && last.m_pElement != nullptr, "An iterator is invalid.");

  while (first != last)
    first = Remove(first);

  return last;
}

/*! If uiNewSize is smaller than the size of the list, elements are popped from the back, until the desired size is reached.
    If uiNewSize is larger than the size of the list, default-constructed elements are appended to the list, until the desired size is reached.
*/
template <typename T>
void nsListBase<T>::SetCount(nsUInt32 uiNewSize)
{
  while (m_uiCount > uiNewSize)
    PopBack();

  while (m_uiCount < uiNewSize)
    PushBack();
}

template <typename T>
bool nsListBase<T>::operator==(const nsListBase<T>& rhs) const
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

template <typename T, typename A>
nsList<T, A>::nsList()
  : nsListBase<T>(A::GetAllocator())
{
}

template <typename T, typename A>
nsList<T, A>::nsList(nsAllocator* pAllocator)
  : nsListBase<T>(pAllocator)
{
}

template <typename T, typename A>
nsList<T, A>::nsList(const nsList<T, A>& other)
  : nsListBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
nsList<T, A>::nsList(const nsListBase<T>& other)
  : nsListBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
void nsList<T, A>::operator=(const nsList<T, A>& rhs)
{
  nsListBase<T>::operator=(rhs);
}

template <typename T, typename A>
void nsList<T, A>::operator=(const nsListBase<T>& rhs)
{
  nsListBase<T>::operator=(rhs);
}
