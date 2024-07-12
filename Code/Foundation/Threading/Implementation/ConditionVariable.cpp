#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/ConditionVariable.h>

void nsConditionVariable::Lock()
{
  m_Mutex.Lock();
  ++m_iLockCount;
}

nsResult nsConditionVariable::TryLock()
{
  if (m_Mutex.TryLock().Succeeded())
  {
    ++m_iLockCount;
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

void nsConditionVariable::Unlock()
{
  NS_ASSERT_DEV(m_iLockCount > 0, "Cannot unlock a thread-signal that was not locked before.");
  --m_iLockCount;
  m_Mutex.Unlock();
}
