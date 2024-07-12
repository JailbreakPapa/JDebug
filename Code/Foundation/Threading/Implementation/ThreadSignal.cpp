#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/ThreadSignal.h>

nsThreadSignal::nsThreadSignal(Mode mode /*= Mode::AutoReset*/)
{
  m_Mode = mode;
}

nsThreadSignal::~nsThreadSignal() = default;

void nsThreadSignal::WaitForSignal() const
{
  NS_LOCK(m_ConditionVariable);

  while (!m_bSignalState)
  {
    m_ConditionVariable.UnlockWaitForSignalAndLock();
  }

  if (m_Mode == Mode::AutoReset)
  {
    m_bSignalState = false;
  }
}

nsThreadSignal::WaitResult nsThreadSignal::WaitForSignal(nsTime timeout) const
{
  NS_LOCK(m_ConditionVariable);

  const nsTime tStart = nsTime::Now();
  nsTime tElapsed = nsTime::MakeZero();

  while (!m_bSignalState)
  {
    if (m_ConditionVariable.UnlockWaitForSignalAndLock(timeout - tElapsed) == nsConditionVariable::WaitResult::Timeout)
    {
      return WaitResult::Timeout;
    }

    tElapsed = nsTime::Now() - tStart;
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

void nsThreadSignal::RaiseSignal()
{
  {
    NS_LOCK(m_ConditionVariable);
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

void nsThreadSignal::ClearSignal()
{
  NS_LOCK(m_ConditionVariable);
  m_bSignalState = false;
}
