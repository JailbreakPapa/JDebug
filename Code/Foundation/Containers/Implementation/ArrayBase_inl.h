
template <typename T, typename Derived>
nsArrayBase<T, Derived>::nsArrayBase() = default;

template <typename T, typename Derived>
nsArrayBase<T, Derived>::~nsArrayBase()
{
  NS_ASSERT_DEBUG(m_uiCount == 0, "The derived class did not destruct all objects. Count is {0}.", m_uiCount);
  NS_ASSERT_DEBUG(m_pElements == nullptr, "The derived class did not free its memory.");
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::operator=(const nsArrayPtr<const T>& rhs)
{
  if (this->GetData() == rhs.GetPtr())
  {
    if (m_uiCount == rhs.GetCount())
      return;

    NS_ASSERT_DEV(m_uiCount > rhs.GetCount(), "Dangling array pointer. The given array pointer points to invalid memory.");
    T* pElements = static_cast<Derived*>(this)->GetElementsPtr();
    nsMemoryUtils::Destruct(pElements + rhs.GetCount(), m_uiCount - rhs.GetCount());
    m_uiCount = rhs.GetCount();
    return;
  }

  const nsUInt32 uiOldCount = m_uiCount;
  const nsUInt32 uiNewCount = rhs.GetCount();

  if (uiNewCount > uiOldCount)
  {
    static_cast<Derived*>(this)->Reserve(uiNewCount);
    T* pElements = static_cast<Derived*>(this)->GetElementsPtr();
    nsMemoryUtils::Copy(pElements, rhs.GetPtr(), uiOldCount);
    nsMemoryUtils::CopyConstructArray(pElements + uiOldCount, rhs.GetPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else
  {
    T* pElements = static_cast<Derived*>(this)->GetElementsPtr();
    nsMemoryUtils::Copy(pElements, rhs.GetPtr(), uiNewCount);
    nsMemoryUtils::Destruct(pElements + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiNewCount;
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE nsArrayBase<T, Derived>::operator nsArrayPtr<const T>() const
{
  return nsArrayPtr<const T>(static_cast<const Derived*>(this)->GetElementsPtr(), m_uiCount);
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE nsArrayBase<T, Derived>::operator nsArrayPtr<T>()
{
  return nsArrayPtr<T>(static_cast<Derived*>(this)->GetElementsPtr(), m_uiCount);
}

template <typename T, typename Derived>
bool nsArrayBase<T, Derived>::operator==(const nsArrayBase<T, Derived>& rhs) const
{
  if (m_uiCount != rhs.GetCount())
    return false;

  return nsMemoryUtils::IsEqual(static_cast<const Derived*>(this)->GetElementsPtr(), rhs.GetData(), m_uiCount);
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE bool nsArrayBase<T, Derived>::operator<(const nsArrayBase<T, Derived>& rhs) const
{
  return GetArrayPtr() < rhs.GetArrayPtr();
}

#if NS_DISABLED(NS_USE_CPP20_OPERATORS)
template <typename T, typename Derived>
bool nsArrayBase<T, Derived>::operator==(const nsArrayPtr<const T>& rhs) const
{
  if (m_uiCount != rhs.GetCount())
    return false;

  return nsMemoryUtils::IsEqual(static_cast<const Derived*>(this)->GetElementsPtr(), rhs.GetPtr(), m_uiCount);
}
#endif

template <typename T, typename Derived>
NS_ALWAYS_INLINE bool nsArrayBase<T, Derived>::operator<(const nsArrayPtr<const T>& rhs) const
{
  return GetArrayPtr() < rhs;
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE const T& nsArrayBase<T, Derived>::operator[](const nsUInt32 uiIndex) const
{
  NS_ASSERT_DEBUG(uiIndex < m_uiCount, "Out of bounds access. Array has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return static_cast<const Derived*>(this)->GetElementsPtr()[uiIndex];
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE T& nsArrayBase<T, Derived>::operator[](const nsUInt32 uiIndex)
{
  NS_ASSERT_DEBUG(uiIndex < m_uiCount, "Out of bounds access. Array has {0} elements, trying to access element at index {1}.", m_uiCount, uiIndex);
  return static_cast<Derived*>(this)->GetElementsPtr()[uiIndex];
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::SetCount(nsUInt32 uiCount)
{
  const nsUInt32 uiOldCount = m_uiCount;
  const nsUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    static_cast<Derived*>(this)->Reserve(uiNewCount);
    nsMemoryUtils::Construct<ConstructAll>(static_cast<Derived*>(this)->GetElementsPtr() + uiOldCount, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    nsMemoryUtils::Destruct(static_cast<Derived*>(this)->GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::SetCount(nsUInt32 uiCount, const T& fillValue)
{
  const nsUInt32 uiOldCount = m_uiCount;
  const nsUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    static_cast<Derived*>(this)->Reserve(uiNewCount);
    nsMemoryUtils::CopyConstruct(static_cast<Derived*>(this)->GetElementsPtr() + uiOldCount, fillValue, uiNewCount - uiOldCount);
  }
  else if (uiNewCount < uiOldCount)
  {
    nsMemoryUtils::Destruct(static_cast<Derived*>(this)->GetElementsPtr() + uiNewCount, uiOldCount - uiNewCount);
  }

  m_uiCount = uiCount;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::EnsureCount(nsUInt32 uiCount)
{
  if (uiCount > m_uiCount)
  {
    SetCount(uiCount);
  }
}

template <typename T, typename Derived>
template <typename> // Second template needed so that the compiler does only instantiate it when called. Otherwise the static_assert would trigger
// early.
void nsArrayBase<T, Derived>::SetCountUninitialized(nsUInt32 uiCount)
{
  static_assert(nsIsPodType<T>::value == nsTypeIsPod::value, "SetCountUninitialized is only supported for POD types.");
  const nsUInt32 uiOldCount = m_uiCount;
  const nsUInt32 uiNewCount = uiCount;

  if (uiNewCount > uiOldCount)
  {
    static_cast<Derived*>(this)->Reserve(uiNewCount);
    // we already assert above that T is a POD type
    // don't construct anything, leave the memory untouched
  }

  m_uiCount = uiCount;
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE nsUInt32 nsArrayBase<T, Derived>::GetCount() const
{
  return m_uiCount;
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE bool nsArrayBase<T, Derived>::IsEmpty() const
{
  return m_uiCount == 0;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::Clear()
{
  nsMemoryUtils::Destruct(static_cast<Derived*>(this)->GetElementsPtr(), m_uiCount);
  m_uiCount = 0;
}

template <typename T, typename Derived>
bool nsArrayBase<T, Derived>::Contains(const T& value) const
{
  return IndexOf(value) != nsInvalidIndex;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::InsertAt(nsUInt32 uiIndex, const T& value)
{
  NS_ASSERT_DEV(uiIndex <= m_uiCount, "Invalid index. Array has {0} elements, trying to insert element at index {1}.", m_uiCount, uiIndex);

  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  nsMemoryUtils::Prepend(static_cast<Derived*>(this)->GetElementsPtr() + uiIndex, value, m_uiCount - uiIndex);
  m_uiCount++;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::InsertAt(nsUInt32 uiIndex, T&& value)
{
  NS_ASSERT_DEV(uiIndex <= m_uiCount, "Invalid index. Array has {0} elements, trying to insert element at index {1}.", m_uiCount, uiIndex);

  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  nsMemoryUtils::Prepend(static_cast<Derived*>(this)->GetElementsPtr() + uiIndex, std::move(value), m_uiCount - uiIndex);
  m_uiCount++;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::InsertRange(const nsArrayPtr<const T>& range, nsUInt32 uiIndex)
{
  const nsUInt32 uiRangeCount = range.GetCount();
  static_cast<Derived*>(this)->Reserve(m_uiCount + uiRangeCount);

  nsMemoryUtils::Prepend(static_cast<Derived*>(this)->GetElementsPtr() + uiIndex, range.GetPtr(), uiRangeCount, m_uiCount - uiIndex);
  m_uiCount += uiRangeCount;
}

template <typename T, typename Derived>
bool nsArrayBase<T, Derived>::RemoveAndCopy(const T& value)
{
  nsUInt32 uiIndex = IndexOf(value);

  if (uiIndex == nsInvalidIndex)
    return false;

  RemoveAtAndCopy(uiIndex);
  return true;
}

template <typename T, typename Derived>
bool nsArrayBase<T, Derived>::RemoveAndSwap(const T& value)
{
  nsUInt32 uiIndex = IndexOf(value);

  if (uiIndex == nsInvalidIndex)
    return false;

  RemoveAtAndSwap(uiIndex);
  return true;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::RemoveAtAndCopy(nsUInt32 uiIndex, nsUInt32 uiNumElements /*= 1*/)
{
  NS_ASSERT_DEV(uiIndex + uiNumElements <= m_uiCount, "Out of bounds access. Array has {0} elements, trying to remove element at index {1}.", m_uiCount, uiIndex + uiNumElements - 1);

  T* pElements = static_cast<Derived*>(this)->GetElementsPtr();

  m_uiCount -= uiNumElements;
  nsMemoryUtils::RelocateOverlapped(pElements + uiIndex, pElements + uiIndex + uiNumElements, m_uiCount - uiIndex);
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::RemoveAtAndSwap(nsUInt32 uiIndex, nsUInt32 uiNumElements /*= 1*/)
{
  NS_ASSERT_DEV(uiIndex + uiNumElements <= m_uiCount, "Out of bounds access. Array has {0} elements, trying to remove element at index {1}.", m_uiCount, uiIndex + uiNumElements - 1);

  T* pElements = static_cast<Derived*>(this)->GetElementsPtr();

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

template <typename T, typename Derived>
nsUInt32 nsArrayBase<T, Derived>::IndexOf(const T& value, nsUInt32 uiStartIndex) const
{
  const T* pElements = static_cast<const Derived*>(this)->GetElementsPtr();

  for (nsUInt32 i = uiStartIndex; i < m_uiCount; i++)
  {
    if (nsMemoryUtils::IsEqual(pElements + i, &value))
      return i;
  }
  return nsInvalidIndex;
}

template <typename T, typename Derived>
nsUInt32 nsArrayBase<T, Derived>::LastIndexOf(const T& value, nsUInt32 uiStartIndex) const
{
  const T* pElements = static_cast<const Derived*>(this)->GetElementsPtr();

  for (nsUInt32 i = nsMath::Min(uiStartIndex, m_uiCount); i-- > 0;)
  {
    if (nsMemoryUtils::IsEqual(pElements + i, &value))
      return i;
  }
  return nsInvalidIndex;
}

template <typename T, typename Derived>
T& nsArrayBase<T, Derived>::ExpandAndGetRef()
{
  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  T* pElements = static_cast<Derived*>(this)->GetElementsPtr();

  nsMemoryUtils::Construct<SkipTrivialTypes>(pElements + m_uiCount, 1);

  T& ReturnRef = *(pElements + m_uiCount);

  m_uiCount++;

  return ReturnRef;
}

template <typename T, typename Derived>
T* nsArrayBase<T, Derived>::ExpandBy(nsUInt32 uiNumNewItems)
{
  this->SetCount(this->GetCount() + uiNumNewItems);
  return GetArrayPtr().GetEndPtr() - uiNumNewItems;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::PushBack(const T& value)
{
  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  nsMemoryUtils::CopyConstruct(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, value, 1);
  m_uiCount++;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::PushBack(T&& value)
{
  static_cast<Derived*>(this)->Reserve(m_uiCount + 1);

  nsMemoryUtils::MoveConstruct<T>(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, std::move(value));
  m_uiCount++;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::PushBackUnchecked(const T& value)
{
  NS_ASSERT_DEBUG(m_uiCount < m_uiCapacity, "Appending unchecked to array with insufficient capacity.");

  nsMemoryUtils::CopyConstruct(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, value, 1);
  m_uiCount++;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::PushBackUnchecked(T&& value)
{
  NS_ASSERT_DEBUG(m_uiCount < m_uiCapacity, "Appending unchecked to array with insufficient capacity.");

  nsMemoryUtils::MoveConstruct<T>(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, std::move(value));
  m_uiCount++;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::PushBackRange(const nsArrayPtr<const T>& range)
{
  const nsUInt32 uiRangeCount = range.GetCount();
  static_cast<Derived*>(this)->Reserve(m_uiCount + uiRangeCount);

  nsMemoryUtils::CopyConstructArray(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, range.GetPtr(), uiRangeCount);
  m_uiCount += uiRangeCount;
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::PopBack(nsUInt32 uiCountToRemove /* = 1 */)
{
  NS_ASSERT_DEV(m_uiCount >= uiCountToRemove, "Out of bounds access. Array has {0} elements, trying to pop {1} elements.", m_uiCount, uiCountToRemove);

  m_uiCount -= uiCountToRemove;
  nsMemoryUtils::Destruct(static_cast<Derived*>(this)->GetElementsPtr() + m_uiCount, uiCountToRemove);
}

template <typename T, typename Derived>
NS_FORCE_INLINE T& nsArrayBase<T, Derived>::PeekBack()
{
  NS_ASSERT_DEBUG(m_uiCount > 0, "Out of bounds access. Trying to peek into an empty array.");
  return static_cast<Derived*>(this)->GetElementsPtr()[m_uiCount - 1];
}

template <typename T, typename Derived>
NS_FORCE_INLINE const T& nsArrayBase<T, Derived>::PeekBack() const
{
  NS_ASSERT_DEBUG(m_uiCount > 0, "Out of bounds access. Trying to peek into an empty array.");
  return static_cast<const Derived*>(this)->GetElementsPtr()[m_uiCount - 1];
}

template <typename T, typename Derived>
template <typename Comparer>
void nsArrayBase<T, Derived>::Sort(const Comparer& comparer)
{
  if (m_uiCount > 1)
  {
    nsArrayPtr<T> ar = *this;
    nsSorting::QuickSort(ar, comparer);
  }
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::Sort()
{
  if (m_uiCount > 1)
  {
    nsArrayPtr<T> ar = *this;
    nsSorting::QuickSort(ar, nsCompareHelper<T>());
  }
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE T* nsArrayBase<T, Derived>::GetData()
{
  if (IsEmpty())
    return nullptr;

  return static_cast<Derived*>(this)->GetElementsPtr();
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE const T* nsArrayBase<T, Derived>::GetData() const
{
  if (IsEmpty())
    return nullptr;

  return static_cast<const Derived*>(this)->GetElementsPtr();
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE nsArrayPtr<T> nsArrayBase<T, Derived>::GetArrayPtr()
{
  return nsArrayPtr<T>(GetData(), GetCount());
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE nsArrayPtr<const T> nsArrayBase<T, Derived>::GetArrayPtr() const
{
  return nsArrayPtr<const T>(GetData(), GetCount());
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE nsArrayPtr<typename nsArrayPtr<T>::ByteType> nsArrayBase<T, Derived>::GetByteArrayPtr()
{
  return GetArrayPtr().ToByteArray();
}

template <typename T, typename Derived>
NS_ALWAYS_INLINE nsArrayPtr<typename nsArrayPtr<const T>::ByteType> nsArrayBase<T, Derived>::GetByteArrayPtr() const
{
  return GetArrayPtr().ToByteArray();
}

template <typename T, typename Derived>
void nsArrayBase<T, Derived>::DoSwap(nsArrayBase<T, Derived>& other)
{
  nsMath::Swap(this->m_pElements, other.m_pElements);
  nsMath::Swap(this->m_uiCapacity, other.m_uiCapacity);
  nsMath::Swap(this->m_uiCount, other.m_uiCount);
}
