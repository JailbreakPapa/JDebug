#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/ConditionVariable.h>

void wdConditionVariable::Lock()
{
  m_Mutex.Lock();
  ++m_iLockCount;
}

wdResult wdConditionVariable::TryLock()
{
  if (m_Mutex.TryLock().Succeeded())
  {
    ++m_iLockCount;
    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

void wdConditionVariable::Unlock()
{
  WD_ASSERT_DEV(m_iLockCount > 0, "Cannot unlock a thread-signal that was not locked before.");
  --m_iLockCount;
  m_Mutex.Unlock();
}

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Implementation/Win/ConditionVariable_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Threading/Implementation/Posix/ConditionVariable_posix.h>
#else
#  error "Unsupported Platform."
#endif



WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_ConditionVariable);
