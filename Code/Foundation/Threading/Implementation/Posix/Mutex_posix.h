
NS_ALWAYS_INLINE nsMutex::nsMutex()
{
  pthread_mutexattr_t mutexAttributes;
  pthread_mutexattr_init(&mutexAttributes);
  pthread_mutexattr_settype(&mutexAttributes, PTHREAD_MUTEX_RECURSIVE);

  pthread_mutex_init(&m_hHandle, &mutexAttributes);

  pthread_mutexattr_destroy(&mutexAttributes);
}

NS_ALWAYS_INLINE nsMutex::~nsMutex()
{
  pthread_mutex_destroy(&m_hHandle);
}

NS_ALWAYS_INLINE void nsMutex::Lock()
{
  pthread_mutex_lock(&m_hHandle);
  ++m_iLockCount;
}

NS_ALWAYS_INLINE nsResult nsMutex::TryLock()
{
  if (pthread_mutex_trylock(&m_hHandle) == 0)
  {
    ++m_iLockCount;
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}
NS_ALWAYS_INLINE void nsMutex::Unlock()
{
  --m_iLockCount;
  pthread_mutex_unlock(&m_hHandle);
}
