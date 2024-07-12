#pragma once

/// \brief Manages a lock (e.g. a mutex) and ensures that it is properly released as the lock object goes out of scope.
template <typename T>
class nsLock
{
public:
  NS_ALWAYS_INLINE explicit nsLock(T& ref_lock)
    : m_Lock(ref_lock)
  {
    m_Lock.Lock();
  }

  NS_ALWAYS_INLINE ~nsLock() { m_Lock.Unlock(); }

private:
  nsLock();
  nsLock(const nsLock<T>& rhs);
  void operator=(const nsLock<T>& rhs);

  T& m_Lock;
};

/// \brief Shortcut for nsLock<Type> l(lock)
#define NS_LOCK(lock) nsLock<decltype(lock)> NS_CONCAT(l_, NS_SOURCE_LINE)(lock)
