
template <typename T>
NS_ALWAYS_INLINE nsSharedPtr<T>::nsSharedPtr()
{
  m_pInstance = nullptr;
  m_pAllocator = nullptr;
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsSharedPtr<T>::nsSharedPtr(const nsInternal::NewInstance<U>& instance)
{
  m_pInstance = instance.m_pInstance;
  m_pAllocator = instance.m_pAllocator;

  AddReferenceIfValid();
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsSharedPtr<T>::nsSharedPtr(U* pInstance, nsAllocator* pAllocator)
{
  m_pInstance = pInstance;
  m_pAllocator = pAllocator;

  AddReferenceIfValid();
}

template <typename T>
NS_ALWAYS_INLINE nsSharedPtr<T>::nsSharedPtr(const nsSharedPtr<T>& other)
{
  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  AddReferenceIfValid();
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsSharedPtr<T>::nsSharedPtr(const nsSharedPtr<U>& other)
{
  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  AddReferenceIfValid();
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsSharedPtr<T>::nsSharedPtr(nsSharedPtr<U>&& other)
{
  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  other.m_pInstance = nullptr;
  other.m_pAllocator = nullptr;
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsSharedPtr<T>::nsSharedPtr(nsUniquePtr<U>&& other)
{
  m_pInstance = other.Release(m_pAllocator);

  AddReferenceIfValid();
}

template <typename T>
NS_ALWAYS_INLINE nsSharedPtr<T>::nsSharedPtr(std::nullptr_t)
{
  m_pInstance = nullptr;
  m_pAllocator = nullptr;
}

template <typename T>
NS_ALWAYS_INLINE nsSharedPtr<T>::~nsSharedPtr()
{
  ReleaseReferenceIfValid();
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsSharedPtr<T>& nsSharedPtr<T>::operator=(const nsInternal::NewInstance<U>& instance)
{
  ReleaseReferenceIfValid();

  m_pInstance = instance.m_pInstance;
  m_pAllocator = instance.m_pAllocator;

  AddReferenceIfValid();

  return *this;
}

template <typename T>
NS_ALWAYS_INLINE nsSharedPtr<T>& nsSharedPtr<T>::operator=(const nsSharedPtr<T>& other)
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
NS_ALWAYS_INLINE nsSharedPtr<T>& nsSharedPtr<T>::operator=(const nsSharedPtr<U>& other)
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
NS_ALWAYS_INLINE nsSharedPtr<T>& nsSharedPtr<T>::operator=(nsSharedPtr<U>&& other)
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
NS_ALWAYS_INLINE nsSharedPtr<T>& nsSharedPtr<T>::operator=(nsUniquePtr<U>&& other)
{
  ReleaseReferenceIfValid();

  m_pInstance = other.Release(m_pAllocator);

  AddReferenceIfValid();

  return *this;
}

template <typename T>
NS_ALWAYS_INLINE nsSharedPtr<T>& nsSharedPtr<T>::operator=(std::nullptr_t)
{
  ReleaseReferenceIfValid();

  return *this;
}

template <typename T>
NS_ALWAYS_INLINE T* nsSharedPtr<T>::Borrow() const
{
  return m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE void nsSharedPtr<T>::Clear()
{
  ReleaseReferenceIfValid();
}

template <typename T>
NS_ALWAYS_INLINE T& nsSharedPtr<T>::operator*() const
{
  return *m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE T* nsSharedPtr<T>::operator->() const
{
  return m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE nsSharedPtr<T>::operator const T*() const
{
  return m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE nsSharedPtr<T>::operator T*()
{
  return m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE nsSharedPtr<T>::operator bool() const
{
  return m_pInstance != nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator==(const nsSharedPtr<T>& rhs) const
{
  return m_pInstance == rhs.m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator!=(const nsSharedPtr<T>& rhs) const
{
  return m_pInstance != rhs.m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator<(const nsSharedPtr<T>& rhs) const
{
  return m_pInstance < rhs.m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator<=(const nsSharedPtr<T>& rhs) const
{
  return !(rhs < *this);
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator>(const nsSharedPtr<T>& rhs) const
{
  return rhs < *this;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator>=(const nsSharedPtr<T>& rhs) const
{
  return !(*this < rhs);
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator==(std::nullptr_t) const
{
  return m_pInstance == nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator!=(std::nullptr_t) const
{
  return m_pInstance != nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator<(std::nullptr_t) const
{
  return m_pInstance < nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator<=(std::nullptr_t) const
{
  return m_pInstance <= nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator>(std::nullptr_t) const
{
  return m_pInstance > nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsSharedPtr<T>::operator>=(std::nullptr_t) const
{
  return m_pInstance >= nullptr;
}

template <typename T>
NS_ALWAYS_INLINE void nsSharedPtr<T>::AddReferenceIfValid()
{
  if (m_pInstance != nullptr)
  {
    m_pInstance->AddRef();
  }
}

template <typename T>
NS_ALWAYS_INLINE void nsSharedPtr<T>::ReleaseReferenceIfValid()
{
  if (m_pInstance != nullptr)
  {
    if (m_pInstance->ReleaseRef() == 0)
    {
      auto pNonConstInstance = const_cast<typename nsTypeTraits<T>::NonConstType*>(m_pInstance);
      NS_DELETE(m_pAllocator, pNonConstInstance);
    }

    m_pInstance = nullptr;
    m_pAllocator = nullptr;
  }
}
