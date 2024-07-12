
template <typename T>
NS_ALWAYS_INLINE nsUniquePtr<T>::nsUniquePtr() = default;

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsUniquePtr<T>::nsUniquePtr(const nsInternal::NewInstance<U>& instance)
{
  m_pInstance = instance.m_pInstance;
  m_pAllocator = instance.m_pAllocator;
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsUniquePtr<T>::nsUniquePtr(U* pInstance, nsAllocator* pAllocator)
{
  m_pInstance = pInstance;
  m_pAllocator = pAllocator;
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsUniquePtr<T>::nsUniquePtr(nsUniquePtr<U>&& other)
{
  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  other.m_pInstance = nullptr;
  other.m_pAllocator = nullptr;
}

template <typename T>
NS_ALWAYS_INLINE nsUniquePtr<T>::nsUniquePtr(std::nullptr_t)
{
}

template <typename T>
NS_ALWAYS_INLINE nsUniquePtr<T>::~nsUniquePtr()
{
  Clear();
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsUniquePtr<T>& nsUniquePtr<T>::operator=(const nsInternal::NewInstance<U>& instance)
{
  Clear();

  m_pInstance = instance.m_pInstance;
  m_pAllocator = instance.m_pAllocator;

  return *this;
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsUniquePtr<T>& nsUniquePtr<T>::operator=(nsUniquePtr<U>&& other)
{
  Clear();

  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  other.m_pInstance = nullptr;
  other.m_pAllocator = nullptr;

  return *this;
}

template <typename T>
NS_ALWAYS_INLINE nsUniquePtr<T>& nsUniquePtr<T>::operator=(std::nullptr_t)
{
  Clear();

  return *this;
}

template <typename T>
NS_ALWAYS_INLINE T* nsUniquePtr<T>::Release()
{
  T* pInstance = m_pInstance;

  m_pInstance = nullptr;
  m_pAllocator = nullptr;

  return pInstance;
}

template <typename T>
NS_ALWAYS_INLINE T* nsUniquePtr<T>::Release(nsAllocator*& out_pAllocator)
{
  T* pInstance = m_pInstance;
  out_pAllocator = m_pAllocator;

  m_pInstance = nullptr;
  m_pAllocator = nullptr;

  return pInstance;
}

template <typename T>
NS_ALWAYS_INLINE T* nsUniquePtr<T>::Borrow() const
{
  return m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE void nsUniquePtr<T>::Clear()
{
  if (m_pAllocator != nullptr)
  {
    NS_DELETE(m_pAllocator, m_pInstance);
  }

  m_pInstance = nullptr;
  m_pAllocator = nullptr;
}

template <typename T>
NS_ALWAYS_INLINE T& nsUniquePtr<T>::operator*() const
{
  return *m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE T* nsUniquePtr<T>::operator->() const
{
  return m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE nsUniquePtr<T>::operator bool() const
{
  return m_pInstance != nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator==(const nsUniquePtr<T>& rhs) const
{
  return m_pInstance == rhs.m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator!=(const nsUniquePtr<T>& rhs) const
{
  return m_pInstance != rhs.m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator<(const nsUniquePtr<T>& rhs) const
{
  return m_pInstance < rhs.m_pInstance;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator<=(const nsUniquePtr<T>& rhs) const
{
  return !(rhs < *this);
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator>(const nsUniquePtr<T>& rhs) const
{
  return rhs < *this;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator>=(const nsUniquePtr<T>& rhs) const
{
  return !(*this < rhs);
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator==(std::nullptr_t) const
{
  return m_pInstance == nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator!=(std::nullptr_t) const
{
  return m_pInstance != nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator<(std::nullptr_t) const
{
  return m_pInstance < nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator<=(std::nullptr_t) const
{
  return m_pInstance <= nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator>(std::nullptr_t) const
{
  return m_pInstance > nullptr;
}

template <typename T>
NS_ALWAYS_INLINE bool nsUniquePtr<T>::operator>=(std::nullptr_t) const
{
  return m_pInstance >= nullptr;
}

//////////////////////////////////////////////////////////////////////////
// free functions

template <typename T>
NS_ALWAYS_INLINE bool operator==(const nsUniquePtr<T>& lhs, const T* rhs)
{
  return lhs.Borrow() == rhs;
}

template <typename T>
NS_ALWAYS_INLINE bool operator==(const nsUniquePtr<T>& lhs, T* rhs)
{
  return lhs.Borrow() == rhs;
}

template <typename T>
NS_ALWAYS_INLINE bool operator!=(const nsUniquePtr<T>& lhs, const T* rhs)
{
  return lhs.Borrow() != rhs;
}

template <typename T>
NS_ALWAYS_INLINE bool operator!=(const nsUniquePtr<T>& lhs, T* rhs)
{
  return lhs.Borrow() != rhs;
}

template <typename T>
NS_ALWAYS_INLINE bool operator==(const T* lhs, const nsUniquePtr<T>& rhs)
{
  return lhs == rhs.Borrow();
}

template <typename T>
NS_ALWAYS_INLINE bool operator==(T* lhs, const nsUniquePtr<T>& rhs)
{
  return lhs == rhs.Borrow();
}

template <typename T>
NS_ALWAYS_INLINE bool operator!=(const T* lhs, const nsUniquePtr<T>& rhs)
{
  return lhs != rhs.Borrow();
}

template <typename T>
NS_ALWAYS_INLINE bool operator!=(T* lhs, const nsUniquePtr<T>& rhs)
{
  return lhs != rhs.Borrow();
}
