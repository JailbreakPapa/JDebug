#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/ThreadSignal.h>

wdThreadSignal::wdThreadSignal(Mode mode /*= Mode::AutoReset*/)
{
  m_Mode = mode;
}

wdThreadSignal::~wdThreadSignal() = default;

void wdThreadSignal::WaitForSignal() const
{
  WD_LOCK(m_ConditionVariable);

  while (!m_bSignalState)
  {
    m_ConditionVariable.UnlockWaitForSignalAndLock();
  }

  if (m_Mode == Mode::AutoReset)
  {
    m_bSignalState = false;
  }
}

wdThreadSignal::WaitResult wdThreadSignal::WaitForSignal(wdTime timeout) const
{
  WD_LOCK(m_ConditionVariable);

  const wdTime tStart = wdTime::Now();
  wdTime tElapsed = wdTime::Zero();

  while (!m_bSignalState)
  {
    if (m_ConditionVariable.UnlockWaitForSignalAndLock(timeout - tElapsed) == wdConditionVariable::WaitResult::Timeout)
    {
      return WaitResult::Timeout;
    }

    tElapsed = wdTime::Now() - tStart;
    if (tElapsed >= timeout)
    {
      return WaitResult::Timeout;
    }
  }

  if (m_Mode == Mode::AutoReset)
  {
    m_bSignalState = false;
  }

  return WaitResult::Signaled;
}

void wdThreadSignal::RaiseSignal()
{
  {
    WD_LOCK(m_ConditionVariable);
    m_bSignalState = true;
  }

  if (m_Mode == Mode::AutoReset)
  {
    // with auto-reset there is no need to wake up more than one
    m_ConditionVariable.SignalOne();
  }
  else
  {
    m_ConditionVariable.SignalAll();
  }
}

void wdThreadSignal::ClearSignal()
{
  WD_LOCK(m_ConditionVariable);
  m_bSignalState = false;
}

WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_ThreadSignal);
