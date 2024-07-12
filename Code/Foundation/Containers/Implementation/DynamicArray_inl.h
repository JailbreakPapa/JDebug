
template <typename T>
nsDynamicArrayBase<T>::nsDynamicArrayBase(nsAllocator* pAllocator)
  : m_pAllocator(pAllocator)
{
}

template <typename T>
nsDynamicArrayBase<T>::nsDynamicArrayBase(T* pInplaceStorage, nsUInt32 uiCapacity, nsAllocator* pAllocator)
  : m_pAllocator(pAllocator)
{
  m_pAllocator.SetFlags(Storage::External);
  this->m_uiCapacity = uiCapacity;
  this->m_pElements = pInplaceStorage;
}

template <typename T>
nsDynamicArrayBase<T>::nsDynamicArrayBase(const nsDynamicArrayBase<T>& other, nsAllocator* pAllocator)
  : m_pAllocator(pAllocator)
{
  nsArrayBase<T, nsDynamicArrayBase<T>>::operator=((nsArrayPtr<const T>)other); // redirect this to the nsArrayPtr version
}

template <typename T>
nsDynamicArrayBase<T>::nsDynamicArrayBase(nsDynamicArrayBase<T>&& other, nsAllocator* pAllocator)
  : m_pAllocator(pAllocator)
{
  *this = std::move(other);
}

template <typename T>
nsDynamicArrayBase<T>::nsDynamicArrayBase(const nsArrayPtr<const T>& other, nsAllocator* pAllocator)
  : m_pAllocator(pAllocator)
{
  nsArrayBase<T, nsDynamicArrayBase<T>>::operator=(other);
}

template <typename T>
nsDynamicArrayBase<T>::~nsDynamicArrayBase()
{
  this->Clear();

  if (m_pAllocator.GetFlags() == Storage::Owned)
  {
    // only delete our storage, if we own it
    NS_DELETE_RAW_BUFFER(this->m_pAllocator, this->m_pElements);
  }

  this->m_uiCapacity = 0;
  this->m_pElements = nullptr;
}

template <typename T>
NS_ALWAYS_INLINE void nsDynamicArrayBase<T>::operator=(const nsDynamicArrayBase<T>& rhs)
{
  nsArrayBase<T, nsDynamicArrayBase<T>>::operator=((nsArrayPtr<const T>)rhs); // redirect this to the nsArrayPtr version
}

template <typename T>
inline void nsDynamicArrayBase<T>::operator=(nsDynamicArrayBase<T>&& rhs) noexcept
{
  // Clear any existing data (calls destructors if necessary)
  this->Clear();

  if (this->m_pAllocator.GetPtr() == rhs.m_pAllocator.GetPtr() && rhs.m_pAllocator.GetFlags() == Storage::Owned) // only move the storage of rhs, if it owns it
  {
    if (this->m_pAllocator.GetFlags() == Storage::Owned)
    {
      // only delete our storage, if we own it
      NS_DELETE_RAW_BUFFER(this->m_pAllocator, this->m_pElements);
    }

    // we now own this storage
    this->m_pAllocator.SetFlags(Storage::Owned);

    // move the data over from the other array
    this->m_uiCount = rhs.m_uiCount;
    this->m_uiCapacity = rhs.m_uiCapacity;
    this->m_pElements = rhs.m_pElements;

    // reset the other array to not reference the data anymore
    rhs.m_pElements = nullptr;
    rhs.m_uiCount = 0;
    rhs.m_uiCapacity = 0;
  }
  else
  {
    // Ensure we have enough data.
    this->Reserve(rhs.m_uiCount);
    this->m_uiCount = rhs.m_uiCount;

    nsMemoryUtils::RelocateConstruct(
      this->GetElementsPtr(), rhs.GetElementsPtr() /* vital to remap rhs.m_pElements to absolute ptr */, rhs.m_uiCount);

    rhs.m_uiCount = 0;
  }
}

template <typename T>
void nsDynamicArrayBase<T>::Swap(nsDynamicArrayBase<T>& other)
{
  if (this->m_pAllocator.GetFlags() == Storage::External && other.m_pAllocator.GetFlags() == Storage::External)
  {
    constexpr nsUInt32 InplaceStorageSize = 64;

    struct alignas(NS_ALIGNMENT_OF(T)) Tmp
    {
      nsUInt8 m_StaticData[InplaceStorageSize * sizeof(T)];
    };

    const nsUInt32 localSize = this->m_uiCount;
    const nsUInt32 otherLocalSize = other.m_uiCount;

    if (localSize <= InplaceStorageSize && otherLocalSize <= InplaceStorageSize && localSize <= other.m_uiCapacity &&
        otherLocalSize <= this->m_uiCapacity)
    {

      Tmp tmp;
      nsMemoryUtils::RelocateConstruct(reinterpret_cast<T*>(tmp.m_StaticData), this->GetElementsPtr(), localSize);
      nsMemoryUtils::RelocateConstruct(this->GetElementsPtr(), other.GetElementsPtr(), otherLocalSize);
      nsMemoryUtils::RelocateConstruct(other.GetElementsPtr(), reinterpret_cast<T*>(tmp.m_StaticData), localSize);

      nsMath::Swap(this->m_pAllocator, other.m_pAllocator);
      nsMath::Swap(this->m_uiCount, other.m_uiCount);

      return; // successfully swapped in place
    }

    // temp buffer was insufficient -> fallthrough
  }

  if (this->m_pAllocator.GetFlags() == Storage::External)
  {
    // enforce using own storage
    this->Reserve(this->m_uiCapacity + 1);
  }

  if (other.m_pAllocator.GetFlags() == Storage::External)
  {
    // enforce using own storage
    other.Reserve(other.m_uiCapacity + 1);
  }

  // no external storage involved -> swap pointers
  nsMath::Swap(this->m_pAllocator, other.m_pAllocator);
  this->DoSwap(other);
}

template <typename T>
void nsDynamicArrayBase<T>::SetCapacity(nsUInt32 uiCapacity)
{
  // do NOT early out here, it is vital that this function does its thing even if the old capacity would be sufficient

  if (this->m_pAllocator.GetFlags() == Storage::Owned && uiCapacity > this->m_uiCapacity)
  {
    this->m_pElements = NS_EXTEND_RAW_BUFFER(this->m_pAllocator, this->m_pElements, this->m_uiCount, uiCapacity);
  }
  else
  {
    T* pOldElements = GetElementsPtr();

    T* pNewElements = NS_NEW_RAW_BUFFER(this->m_pAllocator, T, uiCapacity);
    nsMemoryUtils::RelocateConstruct(pNewElements, pOldElements, this->m_uiCount);

    if (this->m_pAllocator.GetFlags() == Storage::Owned)
    {
      NS_DELETE_RAW_BUFFER(this->m_pAllocator, pOldElements);
    }

    // after any resize, we definitely own the storage
    this->m_pAllocator.SetFlags(Storage::Owned);
    this->m_pElements = pNewElements;
  }

  this->m_uiCapacity = uiCapacity;
}

template <typename T>
void nsDynamicArrayBase<T>::Reserve(nsUInt32 uiCapacity)
{
  if (this->m_uiCapacity >= uiCapacity)
    return;

  const nsUInt64 uiCurCap64 = static_cast<nsUInt64>(this->m_uiCapacity);
  nsUInt64 uiNewCapacity64 = uiCurCap64 + (uiCurCap64 / 2);

  uiNewCapacity64 = nsMath::Max<nsUInt64>(uiNewCapacity64, uiCapacity);

  constexpr nsUInt64 uiMaxCapacity = 0xFFFFFFFFllu - (CAPACITY_ALIGNMENT - 1);

  // the maximum value must leave room for the capacity alignment computation below (without overflowing the 32 bit range)
  uiNewCapacity64 = nsMath::Min<nsUInt64>(uiNewCapacity64, uiMaxCapacity);

  uiNewCapacity64 = (uiNewCapacity64 + (CAPACITY_ALIGNMENT - 1)) & ~(CAPACITY_ALIGNMENT - 1);

  NS_ASSERT_DEV(uiCapacity <= uiNewCapacity64, "The requested capacity of {} elements exceeds the maximum possible capacity of {} elements.", uiCapacity, uiMaxCapacity);

  SetCapacity(static_cast<nsUInt32>(uiNewCapacity64 & 0xFFFFFFFF));
}

template <typename T>
void nsDynamicArrayBase<T>::Compact()
{
  if (m_pAllocator.GetFlags() == Storage::External)
    return;

  if (this->IsEmpty())
  {
    // completely deallocate all data, if the array is empty.
    NS_DELETE_RAW_BUFFER(this->m_pAllocator, this->m_pElements);
    this->m_uiCapacity = 0;
  }
  else
  {
    const nsUInt32 uiNewCapacity = (this->m_uiCount + (CAPACITY_ALIGNMENT - 1)) & ~(CAPACITY_ALIGNMENT - 1);
    if (this->m_uiCapacity != uiNewCapacity)
      SetCapacity(uiNewCapacity);
  }
}

template <typename T>
NS_ALWAYS_INLINE T* nsDynamicArrayBase<T>::GetElementsPtr()
{
  return this->m_pElements;
}

template <typename T>
NS_ALWAYS_INLINE const T* nsDynamicArrayBase<T>::GetElementsPtr() const
{
  return this->m_pElements;
}

template <typename T>
nsUInt64 nsDynamicArrayBase<T>::GetHeapMemoryUsage() const
{
  if (this->m_pAllocator.GetFlags() == Storage::External)
    return 0;

  return (nsUInt64)this->m_uiCapacity * (nsUInt64)sizeof(T);
}

template <typename T, typename A>
nsDynamicArray<T, A>::nsDynamicArray()
  : nsDynamicArrayBase<T>(A::GetAllocator())
{
}

template <typename T, typename A>
nsDynamicArray<T, A>::nsDynamicArray(nsAllocator* pAllocator)
  : nsDynamicArrayBase<T>(pAllocator)
{
}

template <typename T, typename A>
nsDynamicArray<T, A>::nsDynamicArray(const nsDynamicArray<T, A>& other)
  : nsDynamicArrayBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
nsDynamicArray<T, A>::nsDynamicArray(const nsDynamicArrayBase<T>& other)
  : nsDynamicArrayBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
nsDynamicArray<T, A>::nsDynamicArray(const nsArrayPtr<const T>& other)
  : nsDynamicArrayBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
nsDynamicArray<T, A>::nsDynamicArray(nsDynamicArray<T, A>&& other)
  : nsDynamicArrayBase<T>(std::move(other), other.GetAllocator())
{
}

template <typename T, typename A>
nsDynamicArray<T, A>::nsDynamicArray(nsDynamicArrayBase<T>&& other)
  : nsDynamicArrayBase<T>(std::move(other), other.GetAllocator())
{
}

template <typename T, typename A>
void nsDynamicArray<T, A>::operator=(const nsDynamicArray<T, A>& rhs)
{
  nsDynamicArrayBase<T>::operator=(rhs);
}

template <typename T, typename A>
void nsDynamicArray<T, A>::operator=(const nsDynamicArrayBase<T>& rhs)
{
  nsDynamicArrayBase<T>::operator=(rhs);
}

template <typename T, typename A>
void nsDynamicArray<T, A>::operator=(const nsArrayPtr<const T>& rhs)
{
  nsArrayBase<T, nsDynamicArrayBase<T>>::operator=(rhs);
}

template <typename T, typename A>
void nsDynamicArray<T, A>::operator=(nsDynamicArray<T, A>&& rhs) noexcept
{
  nsDynamicArrayBase<T>::operator=(std::move(rhs));
}

template <typename T, typename A>
void nsDynamicArray<T, A>::operator=(nsDynamicArrayBase<T>&& rhs) noexcept
{
  nsDynamicArrayBase<T>::operator=(std::move(rhs));
}

template <typename T, typename AllocatorWrapper>
nsArrayPtr<const T* const> nsMakeArrayPtr(const nsDynamicArray<T*, AllocatorWrapper>& dynArray)
{
  return nsArrayPtr<const T* const>(dynArray.GetData(), dynArray.GetCount());
}

template <typename T, typename AllocatorWrapper>
nsArrayPtr<const T> nsMakeArrayPtr(const nsDynamicArray<T, AllocatorWrapper>& dynArray)
{
  return nsArrayPtr<const T>(dynArray.GetData(), dynArray.GetCount());
}

template <typename T, typename AllocatorWrapper>
nsArrayPtr<T> nsMakeArrayPtr(nsDynamicArray<T, AllocatorWrapper>& in_dynArray)
{
  return nsArrayPtr<T>(in_dynArray.GetData(), in_dynArray.GetCount());
}
