#pragma once

/// \brief Manages a lock (e.g. a mutex) and ensures that it is properly released as the lock object goes out of scope. The lock/unlock will
/// only be done if the boolean condition is satisfied at scope creation time.
template <typename T>
class nsConditionalLock
{
public:
  NS_ALWAYS_INLINE explicit nsConditionalLock(T& lock, bool bCondition)
    : m_lock(lock)
    , m_bCondition(bCondition)
  {
    if (m_bCondition)
    {
      m_lock.Lock();
    }
  }

  NS_ALWAYS_INLINE ~nsConditionalLock()
  {
    if (m_bCondition)
    {
      m_lock.Unlock();
    }
  }

private:
  nsConditionalLock();
  nsConditionalLock(const nsConditionalLock<T>& rhs);
  void operator=(const nsConditionalLock<T>& rhs);

  T& m_lock;
  bool m_bCondition;
};
