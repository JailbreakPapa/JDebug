
template <typename T, nsUInt16 Size>
nsSmallArrayBase<T, Size>::nsSmallArrayBase() = default;

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsSmallArrayBase<T, Size>::nsSmallArrayBase(const nsSmallArrayBase<T, Size>& other, nsAllocator* pAllocator)
{
  CopyFrom((nsArrayPtr<const T>)other, pAllocator);
  m_uiUserData = other.m_uiUserData;
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsSmallArrayBase<T, Size>::nsSmallArrayBase(const nsArrayPtr<const T>& other, nsAllocator* pAllocator)
{
  CopyFrom(other, pAllocator);
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsSmallArrayBase<T, Size>::nsSmallArrayBase(nsSmallArrayBase<T, Size>&& other, nsAllocator* pAllocator)
{
  MoveFrom(std::move(other), pAllocator);
}

template <typename T, nsUInt16 Size>
NS_FORCE_INLINE nsSmallArrayBase<T, Size>::~nsSmallArrayBase()
{
  NS_ASSERT_DEBUG(m_uiCount == 0, "The derived class did not destruct all objects. Count is {0}.", m_uiCount);
  NS_ASSERT_DEBUG(m_pElements == nullptr, "The derived class did not free its memory.");
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::CopyFrom(const nsArrayPtr<const T>& other, nsAllocator* pAllocator)
{
  NS_ASSERT_DEV(other.GetCount() <= nsSmallInvalidIndex, "Can't copy {} elements to small array. Maximum count is {}", other.GetCount(), nsSmallInvalidIndex);

  if (GetData() == other.GetPtr())
  {
    if (m_uiCount == other.GetCount())
      return;

    NS_ASSERT_DEV(m_uiCount > other.GetCount(), "Dangling array pointer. The given array pointer points to invalid memory.");
    T* pElements = GetElementsPtr();
    nsMemoryUtils::Destruct(pElements + other.GetCount(), m_uiCount - other.GetCount());
    m_uiCount = static_cast<nsUInt16>(other.GetCount());
    return;
  }

  const nsUInt32 uiOldCount = m_uiCount;
  const nsUInt32 uiNewCount = other.GetCount();

  if (uiNewCount > uiOldCount)
  {
    Reserve(static_cast<nsUInt16>(uiNewCount), pAllocator);
    T* pElements = GetElementsPtr();
    nsMemoryUtils::Copy(pElements, other.GetPtr(), uiOldCount);
    nsMemoryUtils::CopyConstructArray(pElements + uiOldCount, other.GetPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else
  {
    T* pElements = GetElementsPtr();
    nsMemoryUtils::Copy(pElements, other.GetPtr(), uiNewCount);
    nsMemoryUtils::Destruct(pElements + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = static_cast<nsUInt16>(uiNewCount);
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::MoveFrom(nsSmallArrayBase<T, Size>&& other, nsAllocator* pAllocator)
{
  Clear();

  if (other.m_uiCapacity > Size)
  {
    if (m_uiCapacity > Size)
    {
      // only delete our own external storage
      NS_DELETE_RAW_BUFFER(pAllocator, m_pElements);
    }

    m_uiCapacity = other.m_uiCapacity;
    m_pElements = other.m_pElements;
  }
  else
  {
    nsMemoryUtils::RelocateConstruct(GetElementsPtr(), other.GetElementsPtr(), other.m_uiCount);
  }

  m_uiCount = other.m_uiCount;
  m_uiUserData = other.m_uiUserData;

  // reset the other array to not reference the data anymore
  other.m_pElements = nullptr;
  other.m_uiCount = 0;
  other.m_uiCapacity = 0;
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsSmallArrayBase<T, Size>::operator nsArrayPtr<const T>() const
{
  return nsArrayPtr<const T>(GetElementsPtr(), m_uiCount);
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsSmallArrayBase<T, Size>::operator nsArrayPtr<T>()
{
  return nsArrayPtr<T>(GetElementsPtr(), m_uiCount);
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE bool nsSmallArrayBase<T, Size>::operator==(const nsSmallArrayBase<T, Size>& rhs) const
{
  return *this == rhs.GetArrayPtr();
}

#if NS_DISABLED(NS_USE_CPP20_OPERATORS)
template <typename T, nsUInt16 Size>
bool nsSmallArrayBase<T, Size>::operator==(const nsArrayPtr<const T>& rhs) const
{
  if (m_uiCount != rhs.GetCount())
    return false;

  return nsMemoryUtils::IsEqual(GetElementsPtr(), rhs.GetPtr(), m_uiCount);
}
#endif

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE const T& nsSmallArrayBase<T, Size>::operator[](const nsUInt32 uiIndex) const
{
  NS_ASSERT_DEBUG(uiIndex < m_uiCount, "Out of bounds access. Array has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return GetElementsPtr()[uiIndex];
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE T& nsSmallArrayBase<T, Size>::operator[](const nsUInt32 uiIndex)
{
  NS_ASSERT_DEBUG(uiIndex < m_uiCount, "Out of bounds access. Array has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return GetElementsPtr()[uiIndex];
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::SetCount(nsUInt16 uiCount, nsAllocator* pAllocator)
{
  const nsUInt32 uiOldCount = m_uiCount;
  const nsUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    Reserve(static_cast<nsUInt16>(uiNewCount), pAllocator);
    nsMemoryUtils::Construct<ConstructAll>(GetElementsPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    nsMemoryUtils::Destruct(GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::SetCount(nsUInt16 uiCount, const T& fillValue, nsAllocator* pAllocator)
{
  const nsUInt32 uiOldCount = m_uiCount;
  const nsUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    Reserve(uiCount, pAllocator);
    nsMemoryUtils::CopyConstruct(GetElementsPtr() + uiOldCount, fillValue, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    nsMemoryUtils::Destruct(GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::EnsureCount(nsUInt16 uiCount, nsAllocator* pAllocator)
{
  if (uiCount > m_uiCount)
  {
    SetCount(uiCount, pAllocator);
  }
}

template <typename T, nsUInt16 Size>
template <typename> // Second template needed so that the compiler does only instantiate it when called. Otherwise the static_assert would trigger early.
void nsSmallArrayBase<T, Size>::SetCountUninitialized(nsUInt16 uiCount, nsAllocator* pAllocator)
{
  static_assert(nsIsPodType<T>::value == nsTypeIsPod::value, "SetCountUninitialized is only supported for POD types.");
  const nsUInt16 uiOldCount = m_uiCount;
  const nsUInt16 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    Reserve(uiNewCount, pAllocator);
    nsMemoryUtils::Construct<SkipTrivialTypes>(GetElementsPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    nsMemoryUtils::Destruct(GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsUInt32 nsSmallArrayBase<T, Size>::GetCount() const
{
  return m_uiCount;
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE bool nsSmallArrayBase<T, Size>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::Clear()
{
  nsMemoryUtils::Destruct(GetElementsPtr(), m_uiCount);
  m_uiCount = 0;
}

template <typename T, nsUInt16 Size>
bool nsSmallArrayBase<T, Size>::Contains(const T& value) const
{
  return IndexOf(value) != nsInvalidIndex;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::Insert(const T& value, nsUInt32 uiIndex, nsAllocator* pAllocator)
{
  NS_ASSERT_DEV(uiIndex <= m_uiCount, "Invalid index. Array has {0} elements, trying to insert element at index {1}.", m_uiCount, uiIndex);

  Reserve(m_uiCount + 1, pAllocator);

  nsMemoryUtils::Prepend(GetElementsPtr() + uiIndex, value, m_uiCount - uiIndex);
  m_uiCount++;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::Insert(T&& value, nsUInt32 uiIndex, nsAllocator* pAllocator)
{
  NS_ASSERT_DEV(uiIndex <= m_uiCount, "Invalid index. Array has {0} elements, trying to insert element at index {1}.", m_uiCount, uiIndex);

  Reserve(m_uiCount + 1, pAllocator);

  nsMemoryUtils::Prepend(GetElementsPtr() + uiIndex, std::move(value), m_uiCount - uiIndex);
  m_uiCount++;
}

template <typename T, nsUInt16 Size>
bool nsSmallArrayBase<T, Size>::RemoveAndCopy(const T& value)
{
  nsUInt32 uiIndex = IndexOf(value);

  if (uiIndex == nsInvalidIndex)
    return false;

  RemoveAtAndCopy(uiIndex);
  return true;
}

template <typename T, nsUInt16 Size>
bool nsSmallArrayBase<T, Size>::RemoveAndSwap(const T& value)
{
  nsUInt32 uiIndex = IndexOf(value);

  if (uiIndex == nsInvalidIndex)
    return false;

  RemoveAtAndSwap(uiIndex);
  return true;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::RemoveAtAndCopy(nsUInt32 uiIndex, nsUInt16 uiNumElements /*= 1*/)
{
  NS_ASSERT_DEV(uiIndex + uiNumElements <= m_uiCount, "Out of bounds access. Array has {0} elements, trying to remove element at index {1}.", m_uiCount, uiIndex + uiNumElements - 1);

  T* pElements = GetElementsPtr();

  m_uiCount -= uiNumElements;
  nsMemoryUtils::RelocateOverlapped(pElements + uiIndex, pElements + uiIndex + uiNumElements, m_uiCount - uiIndex);
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::RemoveAtAndSwap(nsUInt32 uiIndex, nsUInt16 uiNumElements /*= 1*/)
{
  NS_ASSERT_DEV(uiIndex + uiNumElements <= m_uiCount, "Out of bounds access. Array has {0} elements, trying to remove element at index {1}.", m_uiCount, uiIndex + uiNumElements - 1);

  T* pElements = GetElementsPtr();

  for (nsUInt32 i = 0; i < uiNumElements; ++i)
  {
    m_uiCount--;

    if (m_uiCount != uiIndex)
    {
      pElements[uiIndex] = std::move(pElements[m_uiCount]);
    }
    nsMemoryUtils::Destruct(pElements + m_uiCount, 1);
    ++uiIndex;
  }
}

template <typename T, nsUInt16 Size>
nsUInt32 nsSmallArrayBase<T, Size>::IndexOf(const T& value, nsUInt32 uiStartIndex) const
{
  const T* pElements = GetElementsPtr();

  for (nsUInt32 i = uiStartIndex; i < m_uiCount; i++)
  {
    if (nsMemoryUtils::IsEqual(pElements + i, &value))
      return i;
  }
  return nsInvalidIndex;
}

template <typename T, nsUInt16 Size>
nsUInt32 nsSmallArrayBase<T, Size>::LastIndexOf(const T& value, nsUInt32 uiStartIndex) const
{
  const T* pElements = GetElementsPtr();

  for (nsUInt32 i = nsMath::Min<nsUInt32>(uiStartIndex, m_uiCount); i-- > 0;)
  {
    if (nsMemoryUtils::IsEqual(pElements + i, &value))
      return i;
  }
  return nsInvalidIndex;
}

template <typename T, nsUInt16 Size>
T& nsSmallArrayBase<T, Size>::ExpandAndGetRef(nsAllocator* pAllocator)
{
  Reserve(m_uiCount + 1, pAllocator);

  T* pElements = GetElementsPtr();

  nsMemoryUtils::Construct<SkipTrivialTypes>(pElements + m_uiCount, 1);

  T& ReturnRef = *(pElements + m_uiCount);

  m_uiCount++;

  return ReturnRef;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::PushBack(const T& value, nsAllocator* pAllocator)
{
  Reserve(m_uiCount + 1, pAllocator);

  nsMemoryUtils::CopyConstruct(GetElementsPtr() + m_uiCount, value, 1);
  m_uiCount++;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::PushBack(T&& value, nsAllocator* pAllocator)
{
  Reserve(m_uiCount + 1, pAllocator);

  nsMemoryUtils::MoveConstruct<T>(GetElementsPtr() + m_uiCount, std::move(value));
  m_uiCount++;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::PushBackUnchecked(const T& value)
{
  NS_ASSERT_DEBUG(m_uiCount < m_uiCapacity, "Appending unchecked to array with insufficient capacity.");

  nsMemoryUtils::CopyConstruct(GetElementsPtr() + m_uiCount, value, 1);
  m_uiCount++;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::PushBackUnchecked(T&& value)
{
  NS_ASSERT_DEBUG(m_uiCount < m_uiCapacity, "Appending unchecked to array with insufficient capacity.");

  nsMemoryUtils::MoveConstruct<T>(GetElementsPtr() + m_uiCount, std::move(value));
  m_uiCount++;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::PushBackRange(const nsArrayPtr<const T>& range, nsAllocator* pAllocator)
{
  const nsUInt32 uiRangeCount = range.GetCount();
  Reserve(m_uiCount + uiRangeCount, pAllocator);

  nsMemoryUtils::CopyConstructArray(GetElementsPtr() + m_uiCount, range.GetPtr(), uiRangeCount);
  m_uiCount += uiRangeCount;
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::PopBack(nsUInt32 uiCountToRemove /* = 1 */)
{
  NS_ASSERT_DEBUG(m_uiCount >= uiCountToRemove, "Out of bounds access. Array has {0} elements, trying to pop {1} elements.", m_uiCount, uiCountToRemove);

  m_uiCount -= uiCountToRemove;
  nsMemoryUtils::Destruct(GetElementsPtr() + m_uiCount, uiCountToRemove);
}

template <typename T, nsUInt16 Size>
NS_FORCE_INLINE T& nsSmallArrayBase<T, Size>::PeekBack()
{
  NS_ASSERT_DEBUG(m_uiCount > 0, "Out of bounds access. Trying to peek into an empty array.");
  return GetElementsPtr()[m_uiCount - 1];
}

template <typename T, nsUInt16 Size>
NS_FORCE_INLINE const T& nsSmallArrayBase<T, Size>::PeekBack() const
{
  NS_ASSERT_DEBUG(m_uiCount > 0, "Out of bounds access. Trying to peek into an empty array.");
  return GetElementsPtr()[m_uiCount - 1];
}

template <typename T, nsUInt16 Size>
template <typename Comparer>
void nsSmallArrayBase<T, Size>::Sort(const Comparer& comparer)
{
  if (m_uiCount > 1)
  {
    nsArrayPtr<T> ar = GetArrayPtr();
    nsSorting::QuickSort(ar, comparer);
  }
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::Sort()
{
  if (m_uiCount > 1)
  {
    nsArrayPtr<T> ar = GetArrayPtr();
    nsSorting::QuickSort(ar, nsCompareHelper<T>());
  }
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE T* nsSmallArrayBase<T, Size>::GetData()
{
  if (IsEmpty())
    return nullptr;

  return GetElementsPtr();
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE const T* nsSmallArrayBase<T, Size>::GetData() const
{
  if (IsEmpty())
    return nullptr;

  return GetElementsPtr();
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsArrayPtr<T> nsSmallArrayBase<T, Size>::GetArrayPtr()
{
  return nsArrayPtr<T>(GetData(), GetCount());
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsArrayPtr<const T> nsSmallArrayBase<T, Size>::GetArrayPtr() const
{
  return nsArrayPtr<const T>(GetData(), GetCount());
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsArrayPtr<typename nsArrayPtr<T>::ByteType> nsSmallArrayBase<T, Size>::GetByteArrayPtr()
{
  return GetArrayPtr().ToByteArray();
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsArrayPtr<typename nsArrayPtr<const T>::ByteType> nsSmallArrayBase<T, Size>::GetByteArrayPtr() const
{
  return GetArrayPtr().ToByteArray();
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::Reserve(nsUInt16 uiCapacity, nsAllocator* pAllocator)
{
  if (m_uiCapacity >= uiCapacity)
    return;

  const nsUInt32 uiCurCap = static_cast<nsUInt32>(m_uiCapacity);
  nsUInt32 uiNewCapacity = uiCurCap + (uiCurCap / 2);

  uiNewCapacity = nsMath::Max<nsUInt32>(uiNewCapacity, uiCapacity);
  uiNewCapacity = nsMemoryUtils::AlignSize<nsUInt32>(uiNewCapacity, CAPACITY_ALIGNMENT);
  uiNewCapacity = nsMath::Min<nsUInt32>(uiNewCapacity, 0xFFFFu);

  SetCapacity(static_cast<nsUInt16>(uiNewCapacity), pAllocator);
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::Compact(nsAllocator* pAllocator)
{
  if (IsEmpty())
  {
    if (m_uiCapacity > Size)
    {
      // completely deallocate all data, if the array is empty.
      NS_DELETE_RAW_BUFFER(pAllocator, m_pElements);
    }

    m_uiCapacity = Size;
    m_pElements = nullptr;
  }
  else if (m_uiCapacity > Size)
  {
    nsUInt32 uiNewCapacity = nsMemoryUtils::AlignSize<nsUInt32>(m_uiCount, CAPACITY_ALIGNMENT);
    uiNewCapacity = nsMath::Min<nsUInt32>(uiNewCapacity, 0xFFFFu);

    if (m_uiCapacity != uiNewCapacity)
      SetCapacity(static_cast<nsUInt16>(uiNewCapacity), pAllocator);
  }
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE nsUInt64 nsSmallArrayBase<T, Size>::GetHeapMemoryUsage() const
{
  return m_uiCapacity <= Size ? 0 : m_uiCapacity * sizeof(T);
}

template <typename T, nsUInt16 Size>
template <typename U>
NS_ALWAYS_INLINE const U& nsSmallArrayBase<T, Size>::GetUserData() const
{
  static_assert(sizeof(U) <= sizeof(nsUInt32));
  return reinterpret_cast<const U&>(m_uiUserData);
}

template <typename T, nsUInt16 Size>
template <typename U>
NS_ALWAYS_INLINE U& nsSmallArrayBase<T, Size>::GetUserData()
{
  static_assert(sizeof(U) <= sizeof(nsUInt32));
  return reinterpret_cast<U&>(m_uiUserData);
}

template <typename T, nsUInt16 Size>
void nsSmallArrayBase<T, Size>::SetCapacity(nsUInt16 uiCapacity, nsAllocator* pAllocator)
{
  if (m_uiCapacity > Size && uiCapacity > m_uiCapacity)
  {
    m_pElements = NS_EXTEND_RAW_BUFFER(pAllocator, m_pElements, m_uiCount, uiCapacity);
    m_uiCapacity = uiCapacity;
  }
  else
  {
    // special case when migrating from in-place to external storage or shrinking
    T* pOldElements = GetElementsPtr();

    const nsUInt32 uiOldCapacity = m_uiCapacity;
    const nsUInt32 uiNewCapacity = uiCapacity;
    m_uiCapacity = nsMath::Max(uiCapacity, Size);

    if (uiNewCapacity > Size)
    {
      // new external storage
      T* pNewElements = NS_NEW_RAW_BUFFER(pAllocator, T, uiCapacity);
      nsMemoryUtils::RelocateConstruct(pNewElements, pOldElements, m_uiCount);
      m_pElements = pNewElements;
    }
    else
    {
      // Re-use inplace storage
      nsMemoryUtils::RelocateConstruct(GetElementsPtr(), pOldElements, m_uiCount);
    }

    if (uiOldCapacity > Size)
    {
      NS_DELETE_RAW_BUFFER(pAllocator, pOldElements);
    }
  }
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE T* nsSmallArrayBase<T, Size>::GetElementsPtr()
{
  return m_uiCapacity <= Size ? reinterpret_cast<T*>(m_StaticData) : m_pElements;
}

template <typename T, nsUInt16 Size>
NS_ALWAYS_INLINE const T* nsSmallArrayBase<T, Size>::GetElementsPtr() const
{
  return m_uiCapacity <= Size ? reinterpret_cast<const T*>(m_StaticData) : m_pElements;
}

//////////////////////////////////////////////////////////////////////////

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
nsSmallArray<T, Size, AllocatorWrapper>::nsSmallArray() = default;

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE nsSmallArray<T, Size, AllocatorWrapper>::nsSmallArray(const nsSmallArray<T, Size, AllocatorWrapper>& other)
  : SUPER(other, AllocatorWrapper::GetAllocator())
{
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE nsSmallArray<T, Size, AllocatorWrapper>::nsSmallArray(const nsArrayPtr<const T>& other)
  : SUPER(other, AllocatorWrapper::GetAllocator())
{
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE nsSmallArray<T, Size, AllocatorWrapper>::nsSmallArray(nsSmallArray<T, Size, AllocatorWrapper>&& other)
  : SUPER(static_cast<SUPER&&>(other), AllocatorWrapper::GetAllocator())
{
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
nsSmallArray<T, Size, AllocatorWrapper>::~nsSmallArray()
{
  SUPER::Clear();
  SUPER::Compact(AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::operator=(const nsSmallArray<T, Size, AllocatorWrapper>& rhs)
{
  *this = ((nsArrayPtr<const T>)rhs); // redirect this to the nsArrayPtr version
  this->m_uiUserData = rhs.m_uiUserData;
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::operator=(const nsArrayPtr<const T>& rhs)
{
  SUPER::CopyFrom(rhs, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::operator=(nsSmallArray<T, Size, AllocatorWrapper>&& rhs) noexcept
{
  SUPER::MoveFrom(std::move(rhs), AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::SetCount(nsUInt16 uiCount)
{
  SUPER::SetCount(uiCount, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::SetCount(nsUInt16 uiCount, const T& fillValue)
{
  SUPER::SetCount(uiCount, fillValue, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::EnsureCount(nsUInt16 uiCount)
{
  SUPER::EnsureCount(uiCount, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
template <typename> // Second template needed so that the compiler does only instantiate it when called. Otherwise the static_assert would trigger early.
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::SetCountUninitialized(nsUInt16 uiCount)
{
  SUPER::SetCountUninitialized(uiCount, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::InsertAt(nsUInt32 uiIndex, const T& value)
{
  SUPER::Insert(value, uiIndex, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::InsertAt(nsUInt32 uiIndex, T&& value)
{
  SUPER::Insert(value, uiIndex, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE T& nsSmallArray<T, Size, AllocatorWrapper>::ExpandAndGetRef()
{
  return SUPER::ExpandAndGetRef(AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::PushBack(const T& value)
{
  SUPER::PushBack(value, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::PushBack(T&& value)
{
  SUPER::PushBack(std::move(value), AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::PushBackRange(const nsArrayPtr<const T>& range)
{
  SUPER::PushBackRange(range, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::Reserve(nsUInt16 uiCapacity)
{
  SUPER::Reserve(uiCapacity, AllocatorWrapper::GetAllocator());
}

template <typename T, nsUInt16 Size, typename AllocatorWrapper /*= nsDefaultAllocatorWrapper*/>
NS_ALWAYS_INLINE void nsSmallArray<T, Size, AllocatorWrapper>::Compact()
{
  SUPER::Compact(AllocatorWrapper::GetAllocator());
}

//////////////////////////////////////////////////////////////////////////

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::iterator begin(nsSmallArrayBase<T, Size>& ref_container)
{
  return ref_container.GetData();
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::const_iterator begin(const nsSmallArrayBase<T, Size>& container)
{
  return container.GetData();
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::const_iterator cbegin(const nsSmallArrayBase<T, Size>& container)
{
  return container.GetData();
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::reverse_iterator rbegin(nsSmallArrayBase<T, Size>& ref_container)
{
  return typename nsSmallArrayBase<T, Size>::reverse_iterator(ref_container.GetData() + ref_container.GetCount() - 1);
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::const_reverse_iterator rbegin(const nsSmallArrayBase<T, Size>& container)
{
  return typename nsSmallArrayBase<T, Size>::const_reverse_iterator(container.GetData() + container.GetCount() - 1);
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::const_reverse_iterator crbegin(const nsSmallArrayBase<T, Size>& container)
{
  return typename nsSmallArrayBase<T, Size>::const_reverse_iterator(container.GetData() + container.GetCount() - 1);
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::iterator end(nsSmallArrayBase<T, Size>& ref_container)
{
  return ref_container.GetData() + ref_container.GetCount();
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::const_iterator end(const nsSmallArrayBase<T, Size>& container)
{
  return container.GetData() + container.GetCount();
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::const_iterator cend(const nsSmallArrayBase<T, Size>& container)
{
  return container.GetData() + container.GetCount();
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::reverse_iterator rend(nsSmallArrayBase<T, Size>& ref_container)
{
  return typename nsSmallArrayBase<T, Size>::reverse_iterator(ref_container.GetData() - 1);
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::const_reverse_iterator rend(const nsSmallArrayBase<T, Size>& container)
{
  return typename nsSmallArrayBase<T, Size>::const_reverse_iterator(container.GetData() - 1);
}

template <typename T, nsUInt16 Size>
typename nsSmallArrayBase<T, Size>::const_reverse_iterator crend(const nsSmallArrayBase<T, Size>& container)
{
  return typename nsSmallArrayBase<T, Size>::const_reverse_iterator(container.GetData() - 1);
}
