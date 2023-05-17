
template <typename T, typename Derived>
wdArrayBase<T, Derived>::wdArrayBase() = default;

template <typename T, typename Derived>
wdArrayBase<T, Derived>::~wdArrayBase()
{
  WD_ASSERT_DEBUG(m_uiCount == 0, "The derived class did not destruct all objects. Count is {0}.", m_uiCount);
  WD_ASSERT_DEBUG(m_pElements == nullptr, "The derived class did not free its memory.");
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::operator=(const wdArrayPtr<const T>& rhs)
{
  if (this->GetData() == rhs.GetPtr())
  {
    if (m_uiCount == rhs.GetCount())
      return;

    WD_ASSERT_DEV(m_uiCount > rhs.GetCount(), "Dangling array pointer. The given array pointer points to invalid memory.");
    T* pElements = static_cast<Derived*>(this)->GetElementsPtr();
    wdMemoryUtils::Destruct(pElements + rhs.GetCount(), m_uiCount - rhs.GetCount());
    m_uiCount = rhs.GetCount();
    return;
  }

  const wdUInt32 uiOldCount = m_uiCount;
  const wdUInt32 uiNewCount = rhs.GetCount();

  if (uiNewCount > uiOldCount)
  {
    static_cast<Derived*>(this)->Reserve(uiNewCount);
    T* pElements = static_cast<Derived*>(this)->GetElementsPtr();
    wdMemoryUtils::Copy(pElements, rhs.GetPtr(), uiOldCount);
    wdMemoryUtils::CopyConstructArray(pElements + uiOldCount, rhs.GetPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else
  {
    T* pElements = static_cast<Derived*>(this)->GetElementsPtr();
    wdMemoryUtils::Copy(pElements, rhs.GetPtr(), uiNewCount);
    wdMemoryUtils::Destruct(pElements + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiNewCount;
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE wdArrayBase<T, Derived>::operator wdArrayPtr<const T>() const
{
  return wdArrayPtr<const T>(static_cast<const Derived*>(this)->GetElementsPtr(), m_uiCount);
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE wdArrayBase<T, Derived>::operator wdArrayPtr<T>()
{
  return wdArrayPtr<T>(static_cast<Derived*>(this)->GetElementsPtr(), m_uiCount);
}

template <typename T, typename Derived>
bool wdArrayBase<T, Derived>::operator==(const wdArrayPtr<const T>& rhs) const
{
  if (m_uiCount != rhs.GetCount())
    return false;

  return wdMemoryUtils::IsEqual(static_cast<const Derived*>(this)->GetElementsPtr(), rhs.GetPtr(), m_uiCount);
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE bool wdArrayBase<T, Derived>::operator!=(const wdArrayPtr<const T>& rhs) const
{
  return !(*this == rhs);
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE bool wdArrayBase<T, Derived>::operator<(const wdArrayPtr<const T>& rhs) const
{
  return GetArrayPtr() < rhs;
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE const T& wdArrayBase<T, Derived>::operator[](const wdUInt32 uiIndex) const
{
  WD_ASSERT_DEV(uiIndex < m_uiCount, "Out of bounds access. Array has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return static_cast<const Derived*>(this)->GetElementsPtr()[uiIndex];
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE T& wdArrayBase<T, Derived>::operator[](const wdUInt32 uiIndex)
{
  WD_ASSERT_DEV(uiIndex < m_uiCount, "Out of bounds access. Array has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return static_cast<Derived*>(this)->GetElementsPtr()[uiIndex];
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::SetCount(wdUInt32 uiCount)
{
  const wdUInt32 uiOldCount = m_uiCount;
  const wdUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    static_cast<Derived*>(this)->Reserve(uiNewCount);
    wdMemoryUtils::DefaultConstruct(static_cast<Derived*>(this)->GetElementsPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    wdMemoryUtils::Destruct(static_cast<Derived*>(this)->GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::SetCount(wdUInt32 uiCount, const T& fillValue)
{
  const wdUInt32 uiOldCount = m_uiCount;
  const wdUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    static_cast<Derived*>(this)->Reserve(uiNewCount);
    wdMemoryUtils::CopyConstruct(static_cast<Derived*>(this)->GetElementsPtr() + uiOldCount, fillValue, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    wdMemoryUtils::Destruct(static_cast<Derived*>(this)->GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::EnsureCount(wdUInt32 uiCount)
{
  if (uiCount > m_uiCount)
  {
    SetCount(uiCount);
  }
}

template <typename T, typename Derived>
template <typename> // Second template needed so that the compiler does only instantiate it when called. Otherwise the static_assert would trigger
// early.
void wdArrayBase<T, Derived>::SetCountUninitialized(wdUInt32 uiCount)
{
  static_assert(wdIsPodType<T>::value == wdTypeIsPod::value, "SetCountUninitialized is only supported for POD types.");
  const wdUInt32 uiOldCount = m_uiCount;
  const wdUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    static_cast<Derived*>(this)->Reserve(uiNewCount);
    wdMemoryUtils::Construct(static_cast<Derived*>(this)->GetElementsPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    wdMemoryUtils::Destruct(static_cast<Derived*>(this)->GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE wdUInt32 wdArrayBase<T, Derived>::GetCount() const
{
  return m_uiCount;
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE bool wdArrayBase<T, Derived>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::Clear()
{
  wdMemoryUtils::Destruct(static_cast<Derived*>(this)->GetElementsPtr(), m_uiCount);
  m_uiCount = 0;
}

template <typename T, typename Derived>
bool wdArrayBase<T, Derived>::Contains(const T& value) const
{
  return IndexOf(value) != wdInvalidIndex;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::Insert(const T& value, wdUInt32 uiIndex)
{
  WD_ASSERT_DEV(uiIndex <= m_uiCount, "Invalid index. Array has {0} elements, trying to insert element at index {1}.", m_uiCount, uiIndex);

  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  wdMemoryUtils::Prepend(static_cast<Derived*>(this)->GetElementsPtr() + uiIndex, value, m_uiCount - uiIndex);
  m_uiCount++;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::Insert(T&& value, wdUInt32 uiIndex)
{
  WD_ASSERT_DEV(uiIndex <= m_uiCount, "Invalid index. Array has {0} elements, trying to insert element at index {1}.", m_uiCount, uiIndex);

  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  wdMemoryUtils::Prepend(static_cast<Derived*>(this)->GetElementsPtr() + uiIndex, std::move(value), m_uiCount - uiIndex);
  m_uiCount++;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::InsertRange(const wdArrayPtr<const T>& range, wdUInt32 uiIndex)
{
  const wdUInt32 uiRangeCount = range.GetCount();
  static_cast<Derived*>(this)->Reserve(m_uiCount + uiRangeCount);

  wdMemoryUtils::Prepend(static_cast<Derived*>(this)->GetElementsPtr() + uiIndex, range.GetPtr(), uiRangeCount, m_uiCount - uiIndex);
  m_uiCount += uiRangeCount;
}

template <typename T, typename Derived>
bool wdArrayBase<T, Derived>::RemoveAndCopy(const T& value)
{
  wdUInt32 uiIndex = IndexOf(value);

  if (uiIndex == wdInvalidIndex)
    return false;

  RemoveAtAndCopy(uiIndex);
  return true;
}

template <typename T, typename Derived>
bool wdArrayBase<T, Derived>::RemoveAndSwap(const T& value)
{
  wdUInt32 uiIndex = IndexOf(value);

  if (uiIndex == wdInvalidIndex)
    return false;

  RemoveAtAndSwap(uiIndex);
  return true;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::RemoveAtAndCopy(wdUInt32 uiIndex, wdUInt32 uiNumElements /*= 1*/)
{
  WD_ASSERT_DEV(uiIndex + uiNumElements <= m_uiCount, "Out of bounds access. Array has {0} elements, trying to remove element at index {1}.",
    m_uiCount, uiIndex + uiNumElements - 1);

  T* pElements = static_cast<Derived*>(this)->GetElementsPtr();

  m_uiCount -= uiNumElements;
  wdMemoryUtils::RelocateOverlapped(pElements + uiIndex, pElements + uiIndex + uiNumElements, m_uiCount - uiIndex);
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::RemoveAtAndSwap(wdUInt32 uiIndex, wdUInt32 uiNumElements /*= 1*/)
{
  WD_ASSERT_DEV(uiIndex + uiNumElements <= m_uiCount, "Out of bounds access. Array has {0} elements, trying to remove element at index {1}.",
    m_uiCount, uiIndex + uiNumElements - 1);

  T* pElements = static_cast<Derived*>(this)->GetElementsPtr();

  for (wdUInt32 i = 0; i < uiNumElements; ++i)
  {
    m_uiCount--;

    if (m_uiCount != uiIndex)
    {
      pElements[uiIndex] = std::move(pElements[m_uiCount]);
    }
    wdMemoryUtils::Destruct(pElements + m_uiCount, 1);
    ++uiIndex;
  }
}

template <typename T, typename Derived>
wdUInt32 wdArrayBase<T, Derived>::IndexOf(const T& value, wdUInt32 uiStartIndex) const
{
  const T* pElements = static_cast<const Derived*>(this)->GetElementsPtr();

  for (wdUInt32 i = uiStartIndex; i < m_uiCount; i++)
  {
    if (wdMemoryUtils::IsEqual(pElements + i, &value))
      return i;
  }
  return wdInvalidIndex;
}

template <typename T, typename Derived>
wdUInt32 wdArrayBase<T, Derived>::LastIndexOf(const T& value, wdUInt32 uiStartIndex) const
{
  const T* pElements = static_cast<const Derived*>(this)->GetElementsPtr();

  for (wdUInt32 i = wdMath::Min(uiStartIndex, m_uiCount); i-- > 0;)
  {
    if (wdMemoryUtils::IsEqual(pElements + i, &value))
      return i;
  }
  return wdInvalidIndex;
}

template <typename T, typename Derived>
T& wdArrayBase<T, Derived>::ExpandAndGetRef()
{
  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  T* pElements = static_cast<Derived*>(this)->GetElementsPtr();

  wdMemoryUtils::Construct(pElements + m_uiCount, 1);

  T& ReturnRef = *(pElements + m_uiCount);

  m_uiCount++;

  return ReturnRef;
}

template <typename T, typename Derived>
T* wdArrayBase<T, Derived>::ExpandBy(wdUInt32 uiNumNewItems)
{
  this->SetCount(this->GetCount() + uiNumNewItems);
  return GetArrayPtr().GetEndPtr() - uiNumNewItems;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::PushBack(const T& value)
{
  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  wdMemoryUtils::CopyConstruct(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, value, 1);
  m_uiCount++;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::PushBack(T&& value)
{
  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  wdMemoryUtils::MoveConstruct<T>(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, std::move(value));
  m_uiCount++;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::PushBackUnchecked(const T& value)
{
  WD_ASSERT_DEV(m_uiCount < m_uiCapacity, "Appending unchecked to array with insufficient capacity.");

  wdMemoryUtils::CopyConstruct(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, value, 1);
  m_uiCount++;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::PushBackUnchecked(T&& value)
{
  WD_ASSERT_DEV(m_uiCount < m_uiCapacity, "Appending unchecked to array with insufficient capacity.");

  wdMemoryUtils::MoveConstruct<T>(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, std::move(value));
  m_uiCount++;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::PushBackRange(const wdArrayPtr<const T>& range)
{
  const wdUInt32 uiRangeCount = range.GetCount();
  static_cast<Derived*>(this)->Reserve(m_uiCount + uiRangeCount);

  wdMemoryUtils::CopyConstructArray(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, range.GetPtr(), uiRangeCount);
  m_uiCount += uiRangeCount;
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::PopBack(wdUInt32 uiCountToRemove /* = 1 */)
{
  WD_ASSERT_DEV(
    m_uiCount >= uiCountToRemove, "Out of bounds access. Array has {0} elements, trying to pop {1} elements.", m_uiCount, uiCountToRemove);

  m_uiCount -= uiCountToRemove;
  wdMemoryUtils::Destruct(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, uiCountToRemove);
}

template <typename T, typename Derived>
WD_FORCE_INLINE T& wdArrayBase<T, Derived>::PeekBack()
{
  WD_ASSERT_DEV(m_uiCount > 0, "Out of bounds access. Trying to peek into an empty array.");
  return static_cast<Derived*>(this)->GetElementsPtr()[m_uiCount - 1];
}

template <typename T, typename Derived>
WD_FORCE_INLINE const T& wdArrayBase<T, Derived>::PeekBack() const
{
  WD_ASSERT_DEV(m_uiCount > 0, "Out of bounds access. Trying to peek into an empty array.");
  return static_cast<const Derived*>(this)->GetElementsPtr()[m_uiCount - 1];
}

template <typename T, typename Derived>
template <typename Comparer>
void wdArrayBase<T, Derived>::Sort(const Comparer& comparer)
{
  if (m_uiCount > 1)
  {
    wdArrayPtr<T> ar = *this;
    wdSorting::QuickSort(ar, comparer);
  }
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::Sort()
{
  if (m_uiCount > 1)
  {
    wdArrayPtr<T> ar = *this;
    wdSorting::QuickSort(ar, wdCompareHelper<T>());
  }
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE T* wdArrayBase<T, Derived>::GetData()
{
  if (IsEmpty())
    return nullptr;

  return static_cast<Derived*>(this)->GetElementsPtr();
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE const T* wdArrayBase<T, Derived>::GetData() const
{
  if (IsEmpty())
    return nullptr;

  return static_cast<const Derived*>(this)->GetElementsPtr();
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE wdArrayPtr<T> wdArrayBase<T, Derived>::GetArrayPtr()
{
  return wdArrayPtr<T>(GetData(), GetCount());
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE wdArrayPtr<const T> wdArrayBase<T, Derived>::GetArrayPtr() const
{
  return wdArrayPtr<const T>(GetData(), GetCount());
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE wdArrayPtr<typename wdArrayPtr<T>::ByteType> wdArrayBase<T, Derived>::GetByteArrayPtr()
{
  return GetArrayPtr().ToByteArray();
}

template <typename T, typename Derived>
WD_ALWAYS_INLINE wdArrayPtr<typename wdArrayPtr<const T>::ByteType> wdArrayBase<T, Derived>::GetByteArrayPtr() const
{
  return GetArrayPtr().ToByteArray();
}

template <typename T, typename Derived>
void wdArrayBase<T, Derived>::DoSwap(wdArrayBase<T, Derived>& other)
{
  wdMath::Swap(this->m_pElements, other.m_pElements);
  wdMath::Swap(this->m_uiCapacity, other.m_uiCapacity);
  wdMath::Swap(this->m_uiCount, other.m_uiCount);
}
