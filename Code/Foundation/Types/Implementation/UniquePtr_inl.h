
template <typename T>
WD_ALWAYS_INLINE wdUniquePtr<T>::wdUniquePtr()
{
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdUniquePtr<T>::wdUniquePtr(const wdInternal::NewInstance<U>& instance)
{
  m_pInstance = instance.m_pInstance;
  m_pAllocator = instance.m_pAllocator;
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdUniquePtr<T>::wdUniquePtr(U* pInstance, wdAllocatorBase* pAllocator)
{
  m_pInstance = pInstance;
  m_pAllocator = pAllocator;
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdUniquePtr<T>::wdUniquePtr(wdUniquePtr<U>&& other)
{
  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  other.m_pInstance = nullptr;
  other.m_pAllocator = nullptr;
}

template <typename T>
WD_ALWAYS_INLINE wdUniquePtr<T>::wdUniquePtr(std::nullptr_t)
{
}

template <typename T>
WD_ALWAYS_INLINE wdUniquePtr<T>::~wdUniquePtr()
{
  Clear();
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdUniquePtr<T>& wdUniquePtr<T>::operator=(const wdInternal::NewInstance<U>& instance)
{
  Clear();

  m_pInstance = instance.m_pInstance;
  m_pAllocator = instance.m_pAllocator;

  return *this;
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdUniquePtr<T>& wdUniquePtr<T>::operator=(wdUniquePtr<U>&& other)
{
  Clear();

  m_pInstance = other.m_pInstance;
  m_pAllocator = other.m_pAllocator;

  other.m_pInstance = nullptr;
  other.m_pAllocator = nullptr;

  return *this;
}

template <typename T>
WD_ALWAYS_INLINE wdUniquePtr<T>& wdUniquePtr<T>::operator=(std::nullptr_t)
{
  Clear();

  return *this;
}

template <typename T>
WD_ALWAYS_INLINE T* wdUniquePtr<T>::Release()
{
  T* pInstance = m_pInstance;

  m_pInstance = nullptr;
  m_pAllocator = nullptr;

  return pInstance;
}

template <typename T>
WD_ALWAYS_INLINE T* wdUniquePtr<T>::Release(wdAllocatorBase*& out_pAllocator)
{
  T* pInstance = m_pInstance;
  out_pAllocator = m_pAllocator;

  m_pInstance = nullptr;
  m_pAllocator = nullptr;

  return pInstance;
}

template <typename T>
WD_ALWAYS_INLINE T* wdUniquePtr<T>::Borrow() const
{
  return m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE void wdUniquePtr<T>::Clear()
{
  WD_DELETE(m_pAllocator, m_pInstance);

  m_pInstance = nullptr;
  m_pAllocator = nullptr;
}

template <typename T>
WD_ALWAYS_INLINE T& wdUniquePtr<T>::operator*() const
{
  return *m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE T* wdUniquePtr<T>::operator->() const
{
  return m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE wdUniquePtr<T>::operator bool() const
{
  return m_pInstance != nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator==(const wdUniquePtr<T>& rhs) const
{
  return m_pInstance == rhs.m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator!=(const wdUniquePtr<T>& rhs) const
{
  return m_pInstance != rhs.m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator<(const wdUniquePtr<T>& rhs) const
{
  return m_pInstance < rhs.m_pInstance;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator<=(const wdUniquePtr<T>& rhs) const
{
  return !(rhs < *this);
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator>(const wdUniquePtr<T>& rhs) const
{
  return rhs < *this;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator>=(const wdUniquePtr<T>& rhs) const
{
  return !(*this < rhs);
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator==(std::nullptr_t) const
{
  return m_pInstance == nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator!=(std::nullptr_t) const
{
  return m_pInstance != nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator<(std::nullptr_t) const
{
  return m_pInstance < nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator<=(std::nullptr_t) const
{
  return m_pInstance <= nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator>(std::nullptr_t) const
{
  return m_pInstance > nullptr;
}

template <typename T>
WD_ALWAYS_INLINE bool wdUniquePtr<T>::operator>=(std::nullptr_t) const
{
  return m_pInstance >= nullptr;
}

//////////////////////////////////////////////////////////////////////////
// free functions

template <typename T>
WD_ALWAYS_INLINE bool operator==(const wdUniquePtr<T>& lhs, const T* rhs)
{
  return lhs.Borrow() == rhs;
}

template <typename T>
WD_ALWAYS_INLINE bool operator==(const wdUniquePtr<T>& lhs, T* rhs)
{
  return lhs.Borrow() == rhs;
}

template <typename T>
WD_ALWAYS_INLINE bool operator!=(const wdUniquePtr<T>& lhs, const T* rhs)
{
  return lhs.Borrow() != rhs;
}

template <typename T>
WD_ALWAYS_INLINE bool operator!=(const wdUniquePtr<T>& lhs, T* rhs)
{
  return lhs.Borrow() != rhs;
}

template <typename T>
WD_ALWAYS_INLINE bool operator==(const T* lhs, const wdUniquePtr<T>& rhs)
{
  return lhs == rhs.Borrow();
}

template <typename T>
WD_ALWAYS_INLINE bool operator==(T* lhs, const wdUniquePtr<T>& rhs)
{
  return lhs == rhs.Borrow();
}

template <typename T>
WD_ALWAYS_INLINE bool operator!=(const T* lhs, const wdUniquePtr<T>& rhs)
{
  return lhs != rhs.Borrow();
}

template <typename T>
WD_ALWAYS_INLINE bool operator!=(T* lhs, const wdUniquePtr<T>& rhs)
{
  return lhs != rhs.Borrow();
}
