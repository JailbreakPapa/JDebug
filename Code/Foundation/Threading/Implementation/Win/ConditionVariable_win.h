#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

static_assert(sizeof(CONDITION_VARIABLE) == sizeof(wdConditionVariableHandle), "not equal!");

wdConditionVariable::wdConditionVariable()
{
  InitializeConditionVariable(reinterpret_cast<CONDITION_VARIABLE*>(&m_Data.m_ConditionVariable));
}

wdConditionVariable::~wdConditionVariable()
{
  WD_ASSERT_DEV(m_iLockCount == 0, "Thread-signal must be unlocked during destruction.");
}

void wdConditionVariable::SignalOne()
{
  WakeConditionVariable(reinterpret_cast<CONDITION_VARIABLE*>(&m_Data.m_ConditionVariable));
}

void wdConditionVariable::SignalAll()
{
  WakeAllConditionVariable(reinterpret_cast<CONDITION_VARIABLE*>(&m_Data.m_ConditionVariable));
}

void wdConditionVariable::UnlockWaitForSignalAndLock() const
{
  WD_ASSERT_DEV(m_iLockCount > 0, "wdConditionVariable must be locked when calling UnlockWaitForSignalAndLock.");

  SleepConditionVariableCS(
    reinterpret_cast<CONDITION_VARIABLE*>(&m_Data.m_ConditionVariable), (CRITICAL_SECTION*)&m_Mutex.GetMutexHandle(), INFINITE);
}

wdConditionVariable::WaitResult wdConditionVariable::UnlockWaitForSignalAndLock(wdTime timeout) const
{
  WD_ASSERT_DEV(m_iLockCount > 0, "wdConditionVariable must be locked when calling UnlockWaitForSignalAndLock.");

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
