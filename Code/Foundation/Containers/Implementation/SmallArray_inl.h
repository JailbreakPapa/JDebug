
template <typename T, wdUInt16 Size>
wdSmallArrayBase<T, Size>::wdSmallArrayBase() = default;

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdSmallArrayBase<T, Size>::wdSmallArrayBase(const wdSmallArrayBase<T, Size>& other, wdAllocatorBase* pAllocator)
{
  CopyFrom((wdArrayPtr<const T>)other, pAllocator);
  m_uiUserData = other.m_uiUserData;
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdSmallArrayBase<T, Size>::wdSmallArrayBase(const wdArrayPtr<const T>& other, wdAllocatorBase* pAllocator)
{
  CopyFrom(other, pAllocator);
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdSmallArrayBase<T, Size>::wdSmallArrayBase(wdSmallArrayBase<T, Size>&& other, wdAllocatorBase* pAllocator)
{
  MoveFrom(std::move(other), pAllocator);
}

template <typename T, wdUInt16 Size>
WD_FORCE_INLINE wdSmallArrayBase<T, Size>::~wdSmallArrayBase()
{
  WD_ASSERT_DEBUG(m_uiCount == 0, "The derived class did not destruct all objects. Count is {0}.", m_uiCount);
  WD_ASSERT_DEBUG(m_pElements == nullptr, "The derived class did not free its memory.");
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::CopyFrom(const wdArrayPtr<const T>& other, wdAllocatorBase* pAllocator)
{
  WD_ASSERT_DEV(other.GetCount() <= wdSmallInvalidIndex, "Can't copy {} elements to small array. Maximum count is {}", other.GetCount(), wdSmallInvalidIndex);

  if (GetData() == other.GetPtr())
  {
    if (m_uiCount == other.GetCount())
      return;

    WD_ASSERT_DEV(m_uiCount > other.GetCount(), "Dangling array pointer. The given array pointer points to invalid memory.");
    T* pElements = GetElementsPtr();
    wdMemoryUtils::Destruct(pElements + other.GetCount(), m_uiCount - other.GetCount());
    m_uiCount = static_cast<wdUInt16>(other.GetCount());
    return;
  }

  const wdUInt32 uiOldCount = m_uiCount;
  const wdUInt32 uiNewCount = other.GetCount();

  if (uiNewCount > uiOldCount)
  {
    Reserve(static_cast<wdUInt16>(uiNewCount), pAllocator);
    T* pElements = GetElementsPtr();
    wdMemoryUtils::Copy(pElements, other.GetPtr(), uiOldCount);
    wdMemoryUtils::CopyConstructArray(pElements + uiOldCount, other.GetPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else
  {
    T* pElements = GetElementsPtr();
    wdMemoryUtils::Copy(pElements, other.GetPtr(), uiNewCount);
    wdMemoryUtils::Destruct(pElements + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = static_cast<wdUInt16>(uiNewCount);
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::MoveFrom(wdSmallArrayBase<T, Size>&& other, wdAllocatorBase* pAllocator)
{
  Clear();

  if (other.m_uiCapacity > Size)
  {
    if (m_uiCapacity > Size)
    {
      // only delete our own external storage
      WD_DELETE_RAW_BUFFER(pAllocator, m_pElements);
    }

    m_uiCapacity = other.m_uiCapacity;
    m_pElements = other.m_pElements;
  }
  else
  {
    wdMemoryUtils::RelocateConstruct(GetElementsPtr(), other.GetElementsPtr(), other.m_uiCount);
  }

  m_uiCount = other.m_uiCount;
  m_uiUserData = other.m_uiUserData;

  // reset the other array to not reference the data anymore
  other.m_pElements = nullptr;
  other.m_uiCount = 0;
  other.m_uiCapacity = 0;
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdSmallArrayBase<T, Size>::operator wdArrayPtr<const T>() const
{
  return wdArrayPtr<const T>(GetElementsPtr(), m_uiCount);
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdSmallArrayBase<T, Size>::operator wdArrayPtr<T>()
{
  return wdArrayPtr<T>(GetElementsPtr(), m_uiCount);
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE bool wdSmallArrayBase<T, Size>::operator==(const wdSmallArrayBase<T, Size>& rhs) const
{
  return *this == rhs.GetArrayPtr();
}

template <typename T, wdUInt16 Size>
bool wdSmallArrayBase<T, Size>::operator==(const wdArrayPtr<const T>& rhs) const
{
  if (m_uiCount != rhs.GetCount())
    return false;

  return wdMemoryUtils::IsEqual(GetElementsPtr(), rhs.GetPtr(), m_uiCount);
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE bool wdSmallArrayBase<T, Size>::operator!=(const wdSmallArrayBase<T, Size>& rhs) const
{
  return !(*this == rhs);
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE bool wdSmallArrayBase<T, Size>::operator!=(const wdArrayPtr<const T>& rhs) const
{
  return !(*this == rhs);
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE const T& wdSmallArrayBase<T, Size>::operator[](const wdUInt32 uiIndex) const
{
  WD_ASSERT_DEV(uiIndex < m_uiCount, "Out of bounds access. Array has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return GetElementsPtr()[uiIndex];
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE T& wdSmallArrayBase<T, Size>::operator[](const wdUInt32 uiIndex)
{
  WD_ASSERT_DEV(uiIndex < m_uiCount, "Out of bounds access. Array has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return GetElementsPtr()[uiIndex];
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::SetCount(wdUInt16 uiCount, wdAllocatorBase* pAllocator)
{
  const wdUInt32 uiOldCount = m_uiCount;
  const wdUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    Reserve(static_cast<wdUInt16>(uiNewCount), pAllocator);
    wdMemoryUtils::DefaultConstruct(GetElementsPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    wdMemoryUtils::Destruct(GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::SetCount(wdUInt16 uiCount, const T& fillValue, wdAllocatorBase* pAllocator)
{
  const wdUInt32 uiOldCount = m_uiCount;
  const wdUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    Reserve(uiCount, pAllocator);
    wdMemoryUtils::CopyConstruct(GetElementsPtr() + uiOldCount, fillValue, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    wdMemoryUtils::Destruct(GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::EnsureCount(wdUInt16 uiCount, wdAllocatorBase* pAllocator)
{
  if (uiCount > m_uiCount)
  {
    SetCount(uiCount, pAllocator);
  }
}

template <typename T, wdUInt16 Size>
template <typename> // Second template needed so that the compiler does only instantiate it when called. Otherwise the static_assert would trigger early.
void wdSmallArrayBase<T, Size>::SetCountUninitialized(wdUInt16 uiCount, wdAllocatorBase* pAllocator)
{
  static_assert(wdIsPodType<T>::value == wdTypeIsPod::value, "SetCountUninitialized is only supported for POD types.");
  const wdUInt32 uiOldCount = m_uiCount;
  const wdUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    Reserve(uiNewCount, pAllocator);
    wdMemoryUtils::Construct(GetElementsPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    wdMemoryUtils::Destruct(GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdUInt32 wdSmallArrayBase<T, Size>::GetCount() const
{
  return m_uiCount;
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE bool wdSmallArrayBase<T, Size>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::Clear()
{
  wdMemoryUtils::Destruct(GetElementsPtr(), m_uiCount);
  m_uiCount = 0;
}

template <typename T, wdUInt16 Size>
bool wdSmallArrayBase<T, Size>::Contains(const T& value) const
{
  return IndexOf(value) != wdInvalidIndex;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::Insert(const T& value, wdUInt32 uiIndex, wdAllocatorBase* pAllocator)
{
  WD_ASSERT_DEV(uiIndex <= m_uiCount, "Invalid index. Array has {0} elements, trying to insert element at index {1}.", m_uiCount, uiIndex);

  Reserve(m_uiCount + 1, pAllocator);

  wdMemoryUtils::Prepend(GetElementsPtr() + uiIndex, value, m_uiCount - uiIndex);
  m_uiCount++;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::Insert(T&& value, wdUInt32 uiIndex, wdAllocatorBase* pAllocator)
{
  WD_ASSERT_DEV(uiIndex <= m_uiCount, "Invalid index. Array has {0} elements, trying to insert element at index {1}.", m_uiCount, uiIndex);

  Reserve(m_uiCount + 1, pAllocator);

  wdMemoryUtils::Prepend(GetElementsPtr() + uiIndex, std::move(value), m_uiCount - uiIndex);
  m_uiCount++;
}

template <typename T, wdUInt16 Size>
bool wdSmallArrayBase<T, Size>::RemoveAndCopy(const T& value)
{
  wdUInt32 uiIndex = IndexOf(value);

  if (uiIndex == wdInvalidIndex)
    return false;

  RemoveAtAndCopy(uiIndex);
  return true;
}

template <typename T, wdUInt16 Size>
bool wdSmallArrayBase<T, Size>::RemoveAndSwap(const T& value)
{
  wdUInt32 uiIndex = IndexOf(value);

  if (uiIndex == wdInvalidIndex)
    return false;

  RemoveAtAndSwap(uiIndex);
  return true;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::RemoveAtAndCopy(wdUInt32 uiIndex, wdUInt32 uiNumElements /*= 1*/)
{
  WD_ASSERT_DEV(uiIndex + uiNumElements <= m_uiCount, "Out of bounds access. Array has {0} elements, trying to remove element at index {1}.", m_uiCount, uiIndex + uiNumElements - 1);

  T* pElements = GetElementsPtr();

  m_uiCount -= uiNumElements;
  wdMemoryUtils::RelocateOverlapped(pElements + uiIndex, pElements + uiIndex + uiNumElements, m_uiCount - uiIndex);
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::RemoveAtAndSwap(wdUInt32 uiIndex, wdUInt32 uiNumElements /*= 1*/)
{
  WD_ASSERT_DEV(uiIndex + uiNumElements <= m_uiCount, "Out of bounds access. Array has {0} elements, trying to remove element at index {1}.", m_uiCount, uiIndex + uiNumElements - 1);

  T* pElements = GetElementsPtr();

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

template <typename T, wdUInt16 Size>
wdUInt32 wdSmallArrayBase<T, Size>::IndexOf(const T& value, wdUInt32 uiStartIndex) const
{
  const T* pElements = GetElementsPtr();

  for (wdUInt32 i = uiStartIndex; i < m_uiCount; i++)
  {
    if (wdMemoryUtils::IsEqual(pElements + i, &value))
      return i;
  }
  return wdInvalidIndex;
}

template <typename T, wdUInt16 Size>
wdUInt32 wdSmallArrayBase<T, Size>::LastIndexOf(const T& value, wdUInt32 uiStartIndex) const
{
  const T* pElements = GetElementsPtr();

  for (wdUInt32 i = wdMath::Min<wdUInt32>(uiStartIndex, m_uiCount); i-- > 0;)
  {
    if (wdMemoryUtils::IsEqual(pElements + i, &value))
      return i;
  }
  return wdInvalidIndex;
}

template <typename T, wdUInt16 Size>
T& wdSmallArrayBase<T, Size>::ExpandAndGetRef(wdAllocatorBase* pAllocator)
{
  Reserve(m_uiCount + 1, pAllocator);

  T* pElements = GetElementsPtr();

  wdMemoryUtils::Construct(pElements + m_uiCount, 1);

  T& ReturnRef = *(pElements + m_uiCount);

  m_uiCount++;

  return ReturnRef;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::PushBack(const T& value, wdAllocatorBase* pAllocator)
{
  Reserve(m_uiCount + 1, pAllocator);

  wdMemoryUtils::CopyConstruct(GetElementsPtr() + m_uiCount, value, 1);
  m_uiCount++;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::PushBack(T&& value, wdAllocatorBase* pAllocator)
{
  Reserve(m_uiCount + 1, pAllocator);

  wdMemoryUtils::MoveConstruct<T>(GetElementsPtr() + m_uiCount, std::move(value));
  m_uiCount++;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::PushBackUnchecked(const T& value)
{
  WD_ASSERT_DEV(m_uiCount < m_uiCapacity, "Appending unchecked to array with insufficient capacity.");

  wdMemoryUtils::CopyConstruct(GetElementsPtr() + m_uiCount, value, 1);
  m_uiCount++;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::PushBackUnchecked(T&& value)
{
  WD_ASSERT_DEV(m_uiCount < m_uiCapacity, "Appending unchecked to array with insufficient capacity.");

  wdMemoryUtils::MoveConstruct<T>(GetElementsPtr() + m_uiCount, std::move(value));
  m_uiCount++;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::PushBackRange(const wdArrayPtr<const T>& range, wdAllocatorBase* pAllocator)
{
  const wdUInt32 uiRangeCount = range.GetCount();
  Reserve(m_uiCount + uiRangeCount, pAllocator);

  wdMemoryUtils::CopyConstructArray(GetElementsPtr() + m_uiCount, range.GetPtr(), uiRangeCount);
  m_uiCount += uiRangeCount;
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::PopBack(wdUInt32 uiCountToRemove /* = 1 */)
{
  WD_ASSERT_DEV(m_uiCount >= uiCountToRemove, "Out of bounds access. Array has {0} elements, trying to pop {1} elements.", m_uiCount, uiCountToRemove);

  m_uiCount -= uiCountToRemove;
  wdMemoryUtils::Destruct(GetElementsPtr() + m_uiCount, uiCountToRemove);
}

template <typename T, wdUInt16 Size>
WD_FORCE_INLINE T& wdSmallArrayBase<T, Size>::PeekBack()
{
  WD_ASSERT_DEV(m_uiCount > 0, "Out of bounds access. Trying to peek into an empty array.");
  return GetElementsPtr()[m_uiCount - 1];
}

template <typename T, wdUInt16 Size>
WD_FORCE_INLINE const T& wdSmallArrayBase<T, Size>::PeekBack() const
{
  WD_ASSERT_DEV(m_uiCount > 0, "Out of bounds access. Trying to peek into an empty array.");
  return GetElementsPtr()[m_uiCount - 1];
}

template <typename T, wdUInt16 Size>
template <typename Comparer>
void wdSmallArrayBase<T, Size>::Sort(const Comparer& comparer)
{
  if (m_uiCount > 1)
  {
    wdArrayPtr<T> ar = GetArrayPtr();
    wdSorting::QuickSort(ar, comparer);
  }
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::Sort()
{
  if (m_uiCount > 1)
  {
    wdArrayPtr<T> ar = GetArrayPtr();
    wdSorting::QuickSort(ar, wdCompareHelper<T>());
  }
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE T* wdSmallArrayBase<T, Size>::GetData()
{
  if (IsEmpty())
    return nullptr;

  return GetElementsPtr();
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE const T* wdSmallArrayBase<T, Size>::GetData() const
{
  if (IsEmpty())
    return nullptr;

  return GetElementsPtr();
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdArrayPtr<T> wdSmallArrayBase<T, Size>::GetArrayPtr()
{
  return wdArrayPtr<T>(GetData(), GetCount());
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdArrayPtr<const T> wdSmallArrayBase<T, Size>::GetArrayPtr() const
{
  return wdArrayPtr<const T>(GetData(), GetCount());
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdArrayPtr<typename wdArrayPtr<T>::ByteType> wdSmallArrayBase<T, Size>::GetByteArrayPtr()
{
  return GetArrayPtr().ToByteArray();
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdArrayPtr<typename wdArrayPtr<const T>::ByteType> wdSmallArrayBase<T, Size>::GetByteArrayPtr() const
{
  return GetArrayPtr().ToByteArray();
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::Reserve(wdUInt16 uiCapacity, wdAllocatorBase* pAllocator)
{
  if (m_uiCapacity >= uiCapacity)
    return;

  const wdUInt32 uiCurCap = static_cast<wdUInt32>(m_uiCapacity);
  wdUInt32 uiNewCapacity = uiCurCap + (uiCurCap / 2);

  uiNewCapacity = wdMath::Max<wdUInt32>(uiNewCapacity, uiCapacity);
  uiNewCapacity = wdMemoryUtils::AlignSize<wdUInt32>(uiNewCapacity, CAPACITY_ALIGNMENT);
  uiNewCapacity = wdMath::Min<wdUInt32>(uiNewCapacity, 0xFFFFu);

  SetCapacity(static_cast<wdUInt16>(uiNewCapacity), pAllocator);
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::Compact(wdAllocatorBase* pAllocator)
{
  if (IsEmpty())
  {
    if (m_uiCapacity > Size)
    {
      // completely deallocate all data, if the array is empty.
      WD_DELETE_RAW_BUFFER(pAllocator, m_pElements);
    }

    m_uiCapacity = Size;
    m_pElements = nullptr;
  }
  else if (m_uiCapacity > Size)
  {
    wdUInt32 uiNewCapacity = wdMemoryUtils::AlignSize<wdUInt32>(m_uiCount, CAPACITY_ALIGNMENT);
    uiNewCapacity = wdMath::Min<wdUInt32>(uiNewCapacity, 0xFFFFu);

    if (m_uiCapacity != uiNewCapacity)
      SetCapacity(static_cast<wdUInt16>(uiNewCapacity), pAllocator);
  }
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE wdUInt64 wdSmallArrayBase<T, Size>::GetHeapMemoryUsage() const
{
  return m_uiCapacity <= Size ? 0 : m_uiCapacity * sizeof(T);
}

template <typename T, wdUInt16 Size>
template <typename U>
WD_ALWAYS_INLINE const U& wdSmallArrayBase<T, Size>::GetUserData() const
{
  static_assert(sizeof(U) <= sizeof(wdUInt32));
  return reinterpret_cast<const U&>(m_uiUserData);
}

template <typename T, wdUInt16 Size>
template <typename U>
WD_ALWAYS_INLINE U& wdSmallArrayBase<T, Size>::GetUserData()
{
  static_assert(sizeof(U) <= sizeof(wdUInt32));
  return reinterpret_cast<U&>(m_uiUserData);
}

template <typename T, wdUInt16 Size>
void wdSmallArrayBase<T, Size>::SetCapacity(wdUInt16 uiCapacity, wdAllocatorBase* pAllocator)
{
  if (m_uiCapacity > Size && uiCapacity > m_uiCapacity)
  {
    m_pElements = WD_EXTEND_RAW_BUFFER(pAllocator, m_pElements, m_uiCount, uiCapacity);
    m_uiCapacity = uiCapacity;
  }
  else
  {
    // special case when migrating from in-place to external storage or shrinking
    T* pOldElements = GetElementsPtr();

    const wdUInt32 uiOldCapacity = m_uiCapacity;
    const wdUInt32 uiNewCapacity = uiCapacity;
    m_uiCapacity = wdMath::Max(uiCapacity, Size);

    if (uiNewCapacity > Size)
    {
      // new external storage
      T* pNewElements = WD_NEW_RAW_BUFFER(pAllocator, T, uiCapacity);
      wdMemoryUtils::RelocateConstruct(pNewElements, pOldElements, m_uiCount);
      m_pElements = pNewElements;
    }
    else
    {
      // Re-use inplace storage
      wdMemoryUtils::RelocateConstruct(GetElementsPtr(), pOldElements, m_uiCount);
    }

    if (uiOldCapacity > Size)
    {
      WD_DELETE_RAW_BUFFER(pAllocator, pOldElements);
    }
  }
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE T* wdSmallArrayBase<T, Size>::GetElementsPtr()
{
  return m_uiCapacity <= Size ? reinterpret_cast<T*>(m_StaticData) : m_pElements;
}

template <typename T, wdUInt16 Size>
WD_ALWAYS_INLINE const T* wdSmallArrayBase<T, Size>::GetElementsPtr() const
{
  return m_uiCapacity <= Size ? reinterpret_cast<const T*>(m_StaticData) : m_pElements;
}

//////////////////////////////////////////////////////////////////////////

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
wdSmallArray<T, Size, AllocatorWrapper>::wdSmallArray() = default;

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE wdSmallArray<T, Size, AllocatorWrapper>::wdSmallArray(const wdSmallArray<T, Size, AllocatorWrapper>& other)
  : SUPER(other, AllocatorWrapper::GetAllocator())
{
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE wdSmallArray<T, Size, AllocatorWrapper>::wdSmallArray(const wdArrayPtr<const T>& other)
  : SUPER(other, AllocatorWrapper::GetAllocator())
{
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE wdSmallArray<T, Size, AllocatorWrapper>::wdSmallArray(wdSmallArray<T, Size, AllocatorWrapper>&& other)
  : SUPER(static_cast<SUPER&&>(other), AllocatorWrapper::GetAllocator())
{
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
wdSmallArray<T, Size, AllocatorWrapper>::~wdSmallArray()
{
  SUPER::Clear();
  SUPER::Compact(AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::operator=(const wdSmallArray<T, Size, AllocatorWrapper>& rhs)
{
  *this = ((wdArrayPtr<const T>)rhs); // redirect this to the wdArrayPtr version
  this->m_uiUserData = rhs.m_uiUserData;
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::operator=(const wdArrayPtr<const T>& rhs)
{
  SUPER::CopyFrom(rhs, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::operator=(wdSmallArray<T, Size, AllocatorWrapper>&& rhs) noexcept
{
  SUPER::MoveFrom(std::move(rhs), AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::SetCount(wdUInt16 uiCount)
{
  SUPER::SetCount(uiCount, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::SetCount(wdUInt16 uiCount, const T& fillValue)
{
  SUPER::SetCount(uiCount, fillValue, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::EnsureCount(wdUInt16 uiCount)
{
  SUPER::EnsureCount(uiCount, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
template <typename> // Second template needed so that the compiler does only instantiate it when called. Otherwise the static_assert would trigger early.
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::SetCountUninitialized(wdUInt16 uiCount)
{
  SUPER::SetCountUninitialized(uiCount, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::Insert(const T& value, wdUInt32 uiIndex)
{
  SUPER::Insert(value, uiIndex, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::Insert(T&& value, wdUInt32 uiIndex)
{
  SUPER::Insert(value, uiIndex, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE T& wdSmallArray<T, Size, AllocatorWrapper>::ExpandAndGetRef()
{
  return SUPER::ExpandAndGetRef(AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::PushBack(const T& value)
{
  SUPER::PushBack(value, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::PushBack(T&& value)
{
  SUPER::PushBack(value, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::PushBackRange(const wdArrayPtr<const T>& range)
{
  SUPER::PushBackRange(range, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::Reserve(wdUInt16 uiCapacity)
{
  SUPER::Reserve(uiCapacity, AllocatorWrapper::GetAllocator());
}

template <typename T, wdUInt16 Size, typename AllocatorWrapper /*= wdDefaultAllocatorWrapper*/>
WD_ALWAYS_INLINE void wdSmallArray<T, Size, AllocatorWrapper>::Compact()
{
  SUPER::Compact(AllocatorWrapper::GetAllocator());
}

//////////////////////////////////////////////////////////////////////////

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::iterator begin(wdSmallArrayBase<T, Size>& ref_container)
{
  return ref_container.GetData();
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::const_iterator begin(const wdSmallArrayBase<T, Size>& container)
{
  return container.GetData();
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::const_iterator cbegin(const wdSmallArrayBase<T, Size>& container)
{
  return container.GetData();
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::reverse_iterator rbegin(wdSmallArrayBase<T, Size>& ref_container)
{
  return typename wdSmallArrayBase<T, Size>::reverse_iterator(ref_container.GetData() + ref_container.GetCount() - 1);
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::const_reverse_iterator rbegin(const wdSmallArrayBase<T, Size>& container)
{
  return typename wdSmallArrayBase<T, Size>::const_reverse_iterator(container.GetData() + container.GetCount() - 1);
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::const_reverse_iterator crbegin(const wdSmallArrayBase<T, Size>& container)
{
  return typename wdSmallArrayBase<T, Size>::const_reverse_iterator(container.GetData() + container.GetCount() - 1);
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::iterator end(wdSmallArrayBase<T, Size>& ref_container)
{
  return ref_container.GetData() + ref_container.GetCount();
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::const_iterator end(const wdSmallArrayBase<T, Size>& container)
{
  return container.GetData() + container.GetCount();
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::const_iterator cend(const wdSmallArrayBase<T, Size>& container)
{
  return container.GetData() + container.GetCount();
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::reverse_iterator rend(wdSmallArrayBase<T, Size>& ref_container)
{
  return typename wdSmallArrayBase<T, Size>::reverse_iterator(ref_container.GetData() - 1);
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::const_reverse_iterator rend(const wdSmallArrayBase<T, Size>& container)
{
  return typename wdSmallArrayBase<T, Size>::const_reverse_iterator(container.GetData() - 1);
}

template <typename T, wdUInt16 Size>
typename wdSmallArrayBase<T, Size>::const_reverse_iterator crend(const wdSmallArrayBase<T, Size>& container)
{
  return typename wdSmallArrayBase<T, Size>::const_reverse_iterator(container.GetData() - 1);
}
