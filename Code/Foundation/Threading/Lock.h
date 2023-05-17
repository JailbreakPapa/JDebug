#pragma once

/// \brief Manages a lock (e.g. a mutex) and ensures that it is properly released as the lock object goes out of scope.
template <typename T>
class wdLock
{
public:
  WD_ALWAYS_INLINE explicit wdLock(T& ref_lock)
    : m_Lock(ref_lock)
  {
    m_Lock.Lock();
  }

  WD_ALWAYS_INLINE ~wdLock() { m_Lock.Unlock(); }

private:
  wdLock();
  wdLock(const wdLock<T>& rhs);
  void operator=(const wdLock<T>& rhs);

  T& m_Lock;
};

/// \brief Shortcut for wdLock<Type> l(lock)
#define WD_LOCK(lock) wdLock<decltype(lock)> WD_CONCAT(l_, WD_SOURCE_LINE)(lock)
