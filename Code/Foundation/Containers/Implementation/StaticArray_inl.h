
template <typename T, wdUInt32 C>
wdStaticArray<T, C>::wdStaticArray()
{
  WD_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
  this->m_uiCapacity = C;
}

template <typename T, wdUInt32 C>
wdStaticArray<T, C>::wdStaticArray(const wdStaticArray<T, C>& rhs)
{
  WD_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
  this->m_uiCapacity = C;
  *this = (wdArrayPtr<const T>)rhs; // redirect this to the wdArrayPtr version
}

template <typename T, wdUInt32 C>
template <wdUInt32 OtherCapacity>
wdStaticArray<T, C>::wdStaticArray(const wdStaticArray<T, OtherCapacity>& rhs)
{
  WD_CHECK_AT_COMPILETIME(OtherCapacity <= C);

  WD_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
  this->m_uiCapacity = C;

  *this = (wdArrayPtr<const T>)rhs; // redirect this to the wdArrayPtr version
}

template <typename T, wdUInt32 C>
wdStaticArray<T, C>::wdStaticArray(const wdArrayPtr<const T>& rhs)
{
  WD_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
  this->m_uiCapacity = C;

  *this = rhs;
}

template <typename T, wdUInt32 C>
wdStaticArray<T, C>::~wdStaticArray()
{
  this->Clear();
  WD_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
}

template <typename T, wdUInt32 C>
WD_ALWAYS_INLINE T* wdStaticArray<T, C>::GetStaticArray()
{
  return reinterpret_cast<T*>(m_Data);
}

template <typename T, wdUInt32 C>
WD_FORCE_INLINE const T* wdStaticArray<T, C>::GetStaticArray() const
{
  return reinterpret_cast<const T*>(m_Data);
}

template <typename T, wdUInt32 C>
WD_FORCE_INLINE void wdStaticArray<T, C>::Reserve(wdUInt32 uiCapacity)
{
  WD_ASSERT_DEV(uiCapacity <= C, "The static array has a fixed capacity of {0}, cannot reserve more elements than that.", C);
  // Nothing to do here
}

template <typename T, wdUInt32 C>
WD_ALWAYS_INLINE void wdStaticArray<T, C>::operator=(const wdStaticArray<T, C>& rhs)
{
  *this = (wdArrayPtr<const T>)rhs; // redirect this to the wdArrayPtr version
}

template <typename T, wdUInt32 C>
template <wdUInt32 OtherCapacity>
WD_ALWAYS_INLINE void wdStaticArray<T, C>::operator=(const wdStaticArray<T, OtherCapacity>& rhs)
{
  *this = (wdArrayPtr<const T>)rhs; // redirect this to the wdArrayPtr version
}

template <typename T, wdUInt32 C>
WD_ALWAYS_INLINE void wdStaticArray<T, C>::operator=(const wdArrayPtr<const T>& rhs)
{
  wdArrayBase<T, wdStaticArray<T, C>>::operator=(rhs);
}

template <typename T, wdUInt32 C>
WD_FORCE_INLINE T* wdStaticArray<T, C>::GetElementsPtr()
{
  return GetStaticArray();
}

template <typename T, wdUInt32 C>
WD_FORCE_INLINE const T* wdStaticArray<T, C>::GetElementsPtr() const
{
  return GetStaticArray();
}
