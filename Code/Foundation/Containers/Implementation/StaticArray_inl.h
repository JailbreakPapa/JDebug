
template <typename T, nsUInt32 C>
nsStaticArray<T, C>::nsStaticArray()
{
  NS_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
  this->m_uiCapacity = C;
}

template <typename T, nsUInt32 C>
nsStaticArray<T, C>::nsStaticArray(const nsStaticArray<T, C>& rhs)
{
  NS_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
  this->m_uiCapacity = C;
  *this = (nsArrayPtr<const T>)rhs; // redirect this to the nsArrayPtr version
}

template <typename T, nsUInt32 C>
template <nsUInt32 OtherCapacity>
nsStaticArray<T, C>::nsStaticArray(const nsStaticArray<T, OtherCapacity>& rhs)
{
  NS_CHECK_AT_COMPILETIME(OtherCapacity <= C);

  NS_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
  this->m_uiCapacity = C;

  *this = (nsArrayPtr<const T>)rhs; // redirect this to the nsArrayPtr version
}

template <typename T, nsUInt32 C>
nsStaticArray<T, C>::nsStaticArray(const nsArrayPtr<const T>& rhs)
{
  NS_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
  this->m_uiCapacity = C;

  *this = rhs;
}

template <typename T, nsUInt32 C>
nsStaticArray<T, C>::~nsStaticArray()
{
  this->Clear();
  NS_ASSERT_DEBUG(this->m_pElements == nullptr, "static arrays should not use m_pElements");
}

template <typename T, nsUInt32 C>
NS_ALWAYS_INLINE T* nsStaticArray<T, C>::GetStaticArray()
{
  return reinterpret_cast<T*>(m_Data);
}

template <typename T, nsUInt32 C>
NS_FORCE_INLINE const T* nsStaticArray<T, C>::GetStaticArray() const
{
  return reinterpret_cast<const T*>(m_Data);
}

template <typename T, nsUInt32 C>
NS_FORCE_INLINE void nsStaticArray<T, C>::Reserve(nsUInt32 uiCapacity)
{
  NS_ASSERT_DEV(uiCapacity <= C, "The static array has a fixed capacity of {0}, cannot reserve more elements than that.", C);
  // Nothing to do here
}

template <typename T, nsUInt32 C>
NS_ALWAYS_INLINE void nsStaticArray<T, C>::operator=(const nsStaticArray<T, C>& rhs)
{
  *this = (nsArrayPtr<const T>)rhs; // redirect this to the nsArrayPtr version
}

template <typename T, nsUInt32 C>
template <nsUInt32 OtherCapacity>
NS_ALWAYS_INLINE void nsStaticArray<T, C>::operator=(const nsStaticArray<T, OtherCapacity>& rhs)
{
  *this = (nsArrayPtr<const T>)rhs; // redirect this to the nsArrayPtr version
}

template <typename T, nsUInt32 C>
NS_ALWAYS_INLINE void nsStaticArray<T, C>::operator=(const nsArrayPtr<const T>& rhs)
{
  nsArrayBase<T, nsStaticArray<T, C>>::operator=(rhs);
}

template <typename T, nsUInt32 C>
NS_FORCE_INLINE T* nsStaticArray<T, C>::GetElementsPtr()
{
  return GetStaticArray();
}

template <typename T, nsUInt32 C>
NS_FORCE_INLINE const T* nsStaticArray<T, C>::GetElementsPtr() const
{
  return GetStaticArray();
}
