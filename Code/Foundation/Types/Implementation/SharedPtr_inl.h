
template <typename T>
WD_ALWAYS_INLINE wdSharedPtr<T>::wdSharedPtr()
{
  m_pInstance = nullptr;
  m_pAllocator = nullptr;
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdSharedPtr<T>::wdSharedPtr(const wdInternal::NewInstance<U>& instance)
{
  m_pInstance = instance.m_pInstance;
  m_pAllocator = instance.m_pAllocator;

  AddReferenceIfValid();
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdSharedPtr<T>::wdSharedPtr(U* pInstance, wdAllocatorBase* pAllocator)
{
  m_pInstance = pInstance;
  m_pAllocator = pAllocator;

  AddReferenceIfValid();
}

template <typename T>
WD_ALWAYS_INLINE wdSharedPtr<T>::wdSharedPtr(const wdSharedPtr<T>& other)
{
  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  AddReferenceIfValid();
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdSharedPtr<T>::wdSharedPtr(const wdSharedPtr<U>& other)
{
  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  AddReferenceIfValid();
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdSharedPtr<T>::wdSharedPtr(wdSharedPtr<U>&& other)
{
  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  other.m_pInstance = nullptr;
  other.m_pAllocator = nullptr;
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdSharedPtr<T>::wdSharedPtr(wdUniquePtr<U>&& other)
{
  m_pInstance = other.Release(m_pAllocator);

  AddReferenceIfValid();
}

template <typename T>
WD_ALWAYS_INLINE wdSharedPtr<T>::wdSharedPtr(std::nullptr_t)
{
  m_pInstance = nullptr;
  m_pAllocator = nullptr;
}

template <typename T>
WD_ALWAYS_INLINE wdSharedPtr<T>::~wdSharedPtr()
{
  ReleaseReferenceIfValid();
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdSharedPtr<T>& wdSharedPtr<T>::operator=(const wdInternal::NewInstance<U>& instance)
{
  ReleaseReferenceIfValid();

  m_pInstance = instance.m_pInstance;
  m_pAllocator = instance.m_pAllocator;

  AddReferenceIfValid();

  return *this;
}

template <typename T>
WD_ALWAYS_INLINE wdSharedPtr<T>& wdSharedPtr<T>::operator=(const wdSharedPtr<T>& other)
{
  if (m_pInstance != other.m_pInstance)
  {
    ReleaseReferenceIfValid();

    m_pInstance = other.m_pInstance;
    m_pAllocator = other.m_pAllocator;

    AddReferenceIfValid();
  }

  return *this;
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdSharedPtr<T>& wdSharedPtr<T>::operator=(const wdSharedPtr<U>& other)
{
  if (m_pInstance != other.m_pInstance)
  {
    ReleaseReferenceIfValid();

    m_pInstance = other.m_pInstance;
    m_pAllocator = other.m_pAllocator;

    AddReferenceIfValid();
  }

  return *this;
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdSharedPtr<T>& wdSharedPtr<T>::operator=(wdSharedPtr<U>&& other)
{
  if (m_pInstance != other.m_pInstance)
  {
    ReleaseReferenceIfValid();

    m_pInstance = other.m_pInstance;
    m_pAllocator = other.m_pAllocator;

    other.m_pInstance = nullptr;
    other.m_pAllocator = nullptr;
  }

  return *this;
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdSharedPtr<T>& wdSharedPtr<T>::operator=(wdUniquePtr<U>&& other)
{
  ReleaseReferenceIfValid();

  m_pInstance = other.Release(m_pAllocator);

  AddReferenceIfValid();

  return *this;
}

template <typename T>
WD_ALWAYS_INLINE wdSharedPtr<T>& wdSharedPtr<T>::operator=(std::nullptr_t)
{
  ReleaseReferenceIfValid();

  return *this;
}

template <typename T>
WD_ALWAYS_INLINE T* wdSharedPtr<T>::Borrow() const
{
  return m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE void wdSharedPtr<T>::Clear()
{
  ReleaseReferenceIfValid();
}

template <typename T>
WD_ALWAYS_INLINE T& wdSharedPtr<T>::operator*() const
{
  return *m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE T* wdSharedPtr<T>::operator->() const
{
  return m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE wdSharedPtr<T>::operator const T*() const
{
  return m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE wdSharedPtr<T>::operator T*()
{
  return m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE wdSharedPtr<T>::operator bool() const
{
  return m_pInstance != nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator==(const wdSharedPtr<T>& rhs) const
{
  return m_pInstance == rhs.m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator!=(const wdSharedPtr<T>& rhs) const
{
  return m_pInstance != rhs.m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator<(const wdSharedPtr<T>& rhs) const
{
  return m_pInstance < rhs.m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator<=(const wdSharedPtr<T>& rhs) const
{
  return !(rhs < *this);
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator>(const wdSharedPtr<T>& rhs) const
{
  return rhs < *this;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator>=(const wdSharedPtr<T>& rhs) const
{
  return !(*this < rhs);
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator==(std::nullptr_t) const
{
  return m_pInstance == nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator!=(std::nullptr_t) const
{
  return m_pInstance != nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator<(std::nullptr_t) const
{
  return m_pInstance < nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator<=(std::nullptr_t) const
{
  return m_pInstance <= nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator>(std::nullptr_t) const
{
  return m_pInstance > nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdSharedPtr<T>::operator>=(std::nullptr_t) const
{
  return m_pInstance >= nullptr;
}

template <typename T>
WD_ALWAYS_INLINE void wdSharedPtr<T>::AddReferenceIfValid()
{
  if (m_pInstance != nullptr)
  {
    m_pInstance->AddRef();
  }
}

template <typename T>
WD_ALWAYS_INLINE void wdSharedPtr<T>::ReleaseReferenceIfValid()
{
  if (m_pInstance != nullptr)
  {
    if (m_pInstance->ReleaseRef() == 0)
    {
      auto pNonConstInstance = const_cast<typename wdTypeTraits<T>::NonConstType*>(m_pInstance);
      WD_DELETE(m_pAllocator, pNonConstInstance);
    }

    m_pInstance = nullptr;
    m_pAllocator = nullptr;
  }
}
