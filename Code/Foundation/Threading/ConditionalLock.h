#pragma once

/// \brief Manages a lock (e.g. a mutex) and ensures that it is properly released as the lock object goes out of scope. The lock/unlock will
/// only be done if the boolean condition is satisfied at scope creation time.
template <typename T>
class wdConditionalLock
{
public:
  WD_ALWAYS_INLINE explicit wdConditionalLock(T& lock, bool bCondition)
    : m_lock(lock)
    , m_bCondition(bCondition)
  {
    if (m_bCondition)
    {
      m_lock.Lock();
    }
  }

  WD_ALWAYS_INLINE ~wdConditionalLock()
  {
    if (m_bCondition)
    {
      m_lock.Unlock();
    }
  }

private:
  wdConditionalLock();
  wdConditionalLock(const wdConditionalLock<T>& rhs);
  void operator=(const wdConditionalLock<T>& rhs);

  T& m_lock;
  bool m_bCondition;
};
