
WD_ALWAYS_INLINE wdMutex::wdMutex()
{
  pthread_mutexattr_t mutexAttributes;
  pthread_mutexattr_init(&mutexAttributes);
  pthread_mutexattr_settype(&mutexAttributes, PTHREAD_MUTEX_RECURSIVE);

  pthread_mutex_init(&m_hHandle, &mutexAttributes);

  pthread_mutexattr_destroy(&mutexAttributes);
}

WD_ALWAYS_INLINE wdMutex::~wdMutex()
{
  pthread_mutex_destroy(&m_hHandle);
}

WD_ALWAYS_INLINE void wdMutex::Lock()
{
  pthread_mutex_lock(&m_hHandle);
  ++m_iLockCount;
}

WD_ALWAYS_INLINE wdResult wdMutex::TryLock()
{
  if (pthread_mutex_trylock(&m_hHandle) == 0)
  {
    ++m_iLockCount;
    return WD_SUCCESS;
  }

  return WD_FAILURE;
}
WD_ALWAYS_INLINE void wdMutex::Unlock()
{
  --m_iLockCount;
  pthread_mutex_unlock(&m_hHandle);
}
