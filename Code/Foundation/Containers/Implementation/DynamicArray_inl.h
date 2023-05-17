
template <typename T>
wdDynamicArrayBase<T>::wdDynamicArrayBase(wdAllocatorBase* pAllocator)
{
  m_pAllocator = pAllocator;
}

template <typename T>
wdDynamicArrayBase<T>::wdDynamicArrayBase(T* pInplaceStorage, wdUInt32 uiCapacity, wdAllocatorBase* pAllocator)
{
  m_pAllocator = pAllocator;
  m_pAllocator.SetFlags(Storage::External);
  this->m_uiCapacity = uiCapacity;
  this->m_pElements = reinterpret_cast<T*>(reinterpret_cast<intptr_t>(pInplaceStorage) - reinterpret_cast<intptr_t>(this)); // store as an offset
}

template <typename T>
wdDynamicArrayBase<T>::wdDynamicArrayBase(const wdDynamicArrayBase<T>& other, wdAllocatorBase* pAllocator)
{
  m_pAllocator = pAllocator;

  wdArrayBase<T, wdDynamicArrayBase<T>>::operator=((wdArrayPtr<const T>)other); // redirect this to the wdArrayPtr version
}

template <typename T>
wdDynamicArrayBase<T>::wdDynamicArrayBase(wdDynamicArrayBase<T>&& other, wdAllocatorBase* pAllocator)
{
  m_pAllocator = pAllocator;

  *this = std::move(other);
}

template <typename T>
wdDynamicArrayBase<T>::wdDynamicArrayBase(const wdArrayPtr<const T>& other, wdAllocatorBase* pAllocator)
{
  m_pAllocator = pAllocator;

  wdArrayBase<T, wdDynamicArrayBase<T>>::operator=(other);
}

template <typename T>
wdDynamicArrayBase<T>::~wdDynamicArrayBase()
{
  this->Clear();

  if (m_pAllocator.GetFlags() == Storage::Owned)
  {
    // only delete our storage, if we own it
    WD_DELETE_RAW_BUFFER(this->m_pAllocator, this->m_pElements);
  }

  this->m_uiCapacity = 0;
  this->m_pElements = nullptr;
}

template <typename T>
WD_ALWAYS_INLINE void wdDynamicArrayBase<T>::operator=(const wdDynamicArrayBase<T>& rhs)
{
  wdArrayBase<T, wdDynamicArrayBase<T>>::operator=((wdArrayPtr<const T>)rhs); // redirect this to the wdArrayPtr version
}

template <typename T>
inline void wdDynamicArrayBase<T>::operator=(wdDynamicArrayBase<T>&& rhs) noexcept
{
  // Clear any existing data (calls destructors if necessary)
  this->Clear();

  if (this->m_pAllocator == rhs.m_pAllocator && rhs.m_pAllocator.GetFlags() == Storage::Owned) // only move the storage of rhs, if it owns it
  {
    if (this->m_pAllocator.GetFlags() == Storage::Owned)
    {
      // only delete our storage, if we own it
      WD_DELETE_RAW_BUFFER(this->m_pAllocator, this->m_pElements);
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

    wdMemoryUtils::RelocateConstruct(
      this->GetElementsPtr(), rhs.GetElementsPtr() /* vital to remap rhs.m_pElements to absolute ptr */, rhs.m_uiCount);

    rhs.m_uiCount = 0;
  }
}

template <typename T>
void wdDynamicArrayBase<T>::Swap(wdDynamicArrayBase<T>& other)
{
  if (this->m_pAllocator.GetFlags() == Storage::External && other.m_pAllocator.GetFlags() == Storage::External)
  {
    constexpr wdUInt32 InplaceStorageSize = 64;

    struct alignas(WD_ALIGNMENT_OF(T)) Tmp
    {
      wdUInt8 m_StaticData[InplaceStorageSize * sizeof(T)];
    };

    const wdUInt32 localSize = this->m_uiCount;
    const wdUInt32 otherLocalSize = other.m_uiCount;

    if (localSize <= InplaceStorageSize && otherLocalSize <= InplaceStorageSize && localSize <= other.m_uiCapacity &&
        otherLocalSize <= this->m_uiCapacity)
    {

      Tmp tmp;
      wdMemoryUtils::RelocateConstruct(reinterpret_cast<T*>(tmp.m_StaticData), this->GetElementsPtr(), localSize);
      wdMemoryUtils::RelocateConstruct(this->GetElementsPtr(), other.GetElementsPtr(), otherLocalSize);
      wdMemoryUtils::RelocateConstruct(other.GetElementsPtr(), reinterpret_cast<T*>(tmp.m_StaticData), localSize);

      wdMath::Swap(this->m_pAllocator, other.m_pAllocator);
      wdMath::Swap(this->m_uiCount, other.m_uiCount);

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
  wdMath::Swap(this->m_pAllocator, other.m_pAllocator);
  this->DoSwap(other);
}

template <typename T>
void wdDynamicArrayBase<T>::SetCapacity(wdUInt32 uiCapacity)
{
  // do NOT early out here, it is vital that this function does its thing even if the old capacity would be sufficient

  if (this->m_pAllocator.GetFlags() == Storage::Owned && uiCapacity > this->m_uiCapacity)
  {
    this->m_pElements = WD_EXTEND_RAW_BUFFER(this->m_pAllocator, this->m_pElements, this->m_uiCount, uiCapacity);
  }
  else
  {
    T* pOldElements = GetElementsPtr();

    T* pNewElements = WD_NEW_RAW_BUFFER(this->m_pAllocator, T, uiCapacity);
    wdMemoryUtils::RelocateConstruct(pNewElements, pOldElements, this->m_uiCount);

    if (this->m_pAllocator.GetFlags() == Storage::Owned)
    {
      WD_DELETE_RAW_BUFFER(this->m_pAllocator, pOldElements);
    }

    // after any resize, we definitely own the storage
    this->m_pAllocator.SetFlags(Storage::Owned);
    this->m_pElements = pNewElements;
  }

  this->m_uiCapacity = uiCapacity;
}

template <typename T>
void wdDynamicArrayBase<T>::Reserve(wdUInt32 uiCapacity)
{
  if (this->m_uiCapacity >= uiCapacity)
    return;

  const wdUInt64 uiCurCap64 = static_cast<wdUInt64>(this->m_uiCapacity);
  wdUInt64 uiNewCapacity64 = uiCurCap64 + (uiCurCap64 / 2);

  uiNewCapacity64 = wdMath::Max<wdUInt64>(uiNewCapacity64, uiCapacity);

  constexpr wdUInt64 uiMaxCapacity = 0xFFFFFFFFllu - (CAPACITY_ALIGNMENT - 1);

  // the maximum value must leave room for the capacity alignment computation below (without overflowing the 32 bit range)
  uiNewCapacity64 = wdMath::Min<wdUInt64>(uiNewCapacity64, uiMaxCapacity);

  uiNewCapacity64 = (uiNewCapacity64 + (CAPACITY_ALIGNMENT - 1)) & ~(CAPACITY_ALIGNMENT - 1);

  WD_ASSERT_DEV(uiCapacity <= uiNewCapacity64, "The requested capacity of {} elements exceeds the maximum possible capacity of {} elements.", uiCapacity, uiMaxCapacity);

  SetCapacity(static_cast<wdUInt32>(uiNewCapacity64 & 0xFFFFFFFF));
}

template <typename T>
void wdDynamicArrayBase<T>::Compact()
{
  if (m_pAllocator.GetFlags() == Storage::External)
    return;

  if (this->IsEmpty())
  {
    // completely deallocate all data, if the array is empty.
    WD_DELETE_RAW_BUFFER(this->m_pAllocator, this->m_pElements);
    this->m_uiCapacity = 0;
  }
  else
  {
    const wdUInt32 uiNewCapacity = (this->m_uiCount + (CAPACITY_ALIGNMENT - 1)) & ~(CAPACITY_ALIGNMENT - 1);
    if (this->m_uiCapacity != uiNewCapacity)
      SetCapacity(uiNewCapacity);
  }
}

template <typename T>
WD_ALWAYS_INLINE T* wdDynamicArrayBase<T>::GetElementsPtr()
{
  if (m_pAllocator.GetFlags() == Storage::External)
  {
    return reinterpret_cast<T*>(reinterpret_cast<intptr_t>(this) + reinterpret_cast<intptr_t>(this->m_pElements));
  }

  return this->m_pElements;
}

template <typename T>
WD_ALWAYS_INLINE const T* wdDynamicArrayBase<T>::GetElementsPtr() const
{
  if (m_pAllocator.GetFlags() == Storage::External)
  {
    return reinterpret_cast<const T*>(reinterpret_cast<intptr_t>(this) + reinterpret_cast<intptr_t>(this->m_pElements));
  }

  return this->m_pElements;
}

template <typename T>
wdUInt64 wdDynamicArrayBase<T>::GetHeapMemoryUsage() const
{
  if (this->m_pAllocator.GetFlags() == Storage::External)
    return 0;

  return (wdUInt64)this->m_uiCapacity * (wdUInt64)sizeof(T);
}

template <typename T, typename A>
wdDynamicArray<T, A>::wdDynamicArray()
  : wdDynamicArrayBase<T>(A::GetAllocator())
{
}

template <typename T, typename A>
wdDynamicArray<T, A>::wdDynamicArray(wdAllocatorBase* pAllocator)
  : wdDynamicArrayBase<T>(pAllocator)
{
}

template <typename T, typename A>
wdDynamicArray<T, A>::wdDynamicArray(const wdDynamicArray<T, A>& other)
  : wdDynamicArrayBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
wdDynamicArray<T, A>::wdDynamicArray(const wdDynamicArrayBase<T>& other)
  : wdDynamicArrayBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
wdDynamicArray<T, A>::wdDynamicArray(const wdArrayPtr<const T>& other)
  : wdDynamicArrayBase<T>(other, A::GetAllocator())
{
}

template <typename T, typename A>
wdDynamicArray<T, A>::wdDynamicArray(wdDynamicArray<T, A>&& other)
  : wdDynamicArrayBase<T>(std::move(other), other.GetAllocator())
{
}

template <typename T, typename A>
wdDynamicArray<T, A>::wdDynamicArray(wdDynamicArrayBase<T>&& other)
  : wdDynamicArrayBase<T>(std::move(other), other.GetAllocator())
{
}

template <typename T, typename A>
void wdDynamicArray<T, A>::operator=(const wdDynamicArray<T, A>& rhs)
{
  wdDynamicArrayBase<T>::operator=(rhs);
}

template <typename T, typename A>
void wdDynamicArray<T, A>::operator=(const wdDynamicArrayBase<T>& rhs)
{
  wdDynamicArrayBase<T>::operator=(rhs);
}

template <typename T, typename A>
void wdDynamicArray<T, A>::operator=(const wdArrayPtr<const T>& rhs)
{
  wdArrayBase<T, wdDynamicArrayBase<T>>::operator=(rhs);
}

template <typename T, typename A>
void wdDynamicArray<T, A>::operator=(wdDynamicArray<T, A>&& rhs) noexcept
{
  wdDynamicArrayBase<T>::operator=(std::move(rhs));
}

template <typename T, typename A>
void wdDynamicArray<T, A>::operator=(wdDynamicArrayBase<T>&& rhs) noexcept
{
  wdDynamicArrayBase<T>::operator=(std::move(rhs));
}

template <typename T, typename AllocatorWrapper>
wdArrayPtr<const T* const> wdMakeArrayPtr(const wdDynamicArray<T*, AllocatorWrapper>& dynArray)
{
  return wdArrayPtr<const T* const>(dynArray.GetData(), dynArray.GetCount());
}

template <typename T, typename AllocatorWrapper>
wdArrayPtr<const T> wdMakeArrayPtr(const wdDynamicArray<T, AllocatorWrapper>& dynArray)
{
  return wdArrayPtr<const T>(dynArray.GetData(), dynArray.GetCount());
}

template <typename T, typename AllocatorWrapper>
wdArrayPtr<T> wdMakeArrayPtr(wdDynamicArray<T, AllocatorWrapper>& in_dynArray)
{
  return wdArrayPtr<T>(in_dynArray.GetData(), in_dynArray.GetCount());
}
