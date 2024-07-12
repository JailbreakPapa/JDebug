#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Threading/ConditionVariable.h>

static_assert(sizeof(CONDITION_VARIABLE) == sizeof(nsConditionVariableHandle), "not equal!");

nsConditionVariable::nsConditionVariable()
{
  InitializeConditionVariable(reinterpret_cast<CONDITION_VARIABLE*>(&m_Data.m_ConditionVariable));
}

nsConditionVariable::~nsConditionVariable()
{
  NS_ASSERT_DEV(m_iLockCount == 0, "Thread-signal must be unlocked during destruction.");
}

void nsConditionVariable::SignalOne()
{
  WakeConditionVariable(reinterpret_cast<CONDITION_VARIABLE*>(&m_Data.m_ConditionVariable));
}

void nsConditionVariable::SignalAll()
{
  WakeAllConditionVariable(reinterpret_cast<CONDITION_VARIABLE*>(&m_Data.m_ConditionVariable));
}

void nsConditionVariable::UnlockWaitForSignalAndLock() const
{
  NS_ASSERT_DEV(m_iLockCount > 0, "nsConditionVariable must be locked when calling UnlockWaitForSignalAndLock.");

  SleepConditionVariableCS(
    reinterpret_cast<CONDITION_VARIABLE*>(&m_Data.m_ConditionVariable), (CRITICAL_SECTION*)&m_Mutex.GetMutexHandle(), INFINITE);
}

nsConditionVariable::WaitResult nsConditionVariable::UnlockWaitForSignalAndLock(nsTime timeout) const
{
  NS_ASSERT_DEV(m_iLockCount > 0, "nsConditionVariable must be locked when calling UnlockWaitForSignalAndLock.");

  // inside the lock
  --m_iLockCount;
  DWORD result = SleepConditionVariableCS(reinterpret_cast<CONDITION_VARIABLE*>(&m_Data.m_ConditionVariable),
    (CRITICAL_SECTION*)&m_Mutex.GetMutexHandle(), static_cast<DWORD>(timeout.GetMilliseconds()));

  if (result == WAIT_TIMEOUT)
  {
    // inside the lock at this point
    ++m_iLockCount;
    return WaitResult::Timeout;
  }

  // inside the lock
  ++m_iLockCount;
  return WaitResult::Signaled;
}

#endif
