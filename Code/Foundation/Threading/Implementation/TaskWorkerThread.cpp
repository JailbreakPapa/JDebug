#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/Implementation/TaskWorkerThread.h>
#include <Foundation/Threading/TaskSystem.h>

thread_local nsTaskWorkerInfo tl_TaskWorkerInfo;

static nsString GenerateThreadName(nsWorkerThreadType::Enum threadType, nsUInt32 uiThreadNumber)
{
  nsStringBuilder sTemp;
  sTemp.SetFormat("{} {}", nsWorkerThreadType::GetThreadTypeName(threadType), uiThreadNumber);
  return sTemp;
}

nsTaskWorkerThread::nsTaskWorkerThread(nsWorkerThreadType::Enum threadType, nsUInt32 uiThreadNumber)
  // We need at least 256 kb of stack size, otherwise the shader compilation tasks will run out of stack space.
  : nsThread(GenerateThreadName(threadType, uiThreadNumber), 256 * 1024)
{
  m_WorkerType = threadType;
  m_uiWorkerThreadNumber = uiThreadNumber & 0xFFFF;
}

nsTaskWorkerThread::~nsTaskWorkerThread() = default;

nsResult nsTaskWorkerThread::DeactivateWorker()
{
  m_bActive = false;

  if (GetThreadStatus() != nsThread::Finished)
  {
    // if necessary, wake this thread up
    WakeUpIfIdle();

    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsUInt32 nsTaskWorkerThread::Run()
{
  NS_ASSERT_DEBUG(
    m_WorkerType != nsWorkerThreadType::Unknown && m_WorkerType != nsWorkerThreadType::MainThread, "Worker threads cannot use this type");
  NS_ASSERT_DEBUG(m_WorkerType < nsWorkerThreadType::ENUM_COUNT, "Worker Thread Type is invalid: {0}", m_WorkerType);

  // once this thread is running, store the worker type in the thread_local variable
  // such that the nsTaskSystem is able to look this up (e.g. in WaitForGroup) to know which types of tasks to help with
  tl_TaskWorkerInfo.m_WorkerType = m_WorkerType;
  tl_TaskWorkerInfo.m_iWorkerIndex = m_uiWorkerThreadNumber;
  tl_TaskWorkerInfo.m_pWorkerState = &m_iWorkerState;

  const bool bIsReserve = m_uiWorkerThreadNumber >= nsTaskSystem::s_pThreadState->m_uiMaxWorkersToUse[m_WorkerType];

  nsTaskPriority::Enum FirstPriority;
  nsTaskPriority::Enum LastPriority;
  nsTaskSystem::DetermineTasksToExecuteOnThread(FirstPriority, LastPriority);

  m_bExecutingTask = false;

  while (m_bActive)
  {
    if (!m_bExecutingTask)
    {
      m_bExecutingTask = true;
      m_StartedWorkingTime = nsTime::Now();
    }

    if (!nsTaskSystem::ExecuteTask(FirstPriority, LastPriority, false, nsTaskGroupID(), &m_iWorkerState))
    {
      WaitForWork();
    }
    else
    {
      ++m_uiNumTasksExecuted;

      if (bIsReserve)
      {
        NS_VERIFY(m_iWorkerState.Set((int)nsTaskWorkerState::Idle) == (int)nsTaskWorkerState::Active, "Corrupt worker state");

        // if this thread is part of the reserve, then don't continue to process tasks indefinitely
        // instead, put this thread to sleep and wake up someone else
        // that someone else may be a thread at the front of the queue, it may also turn out to be this thread again
        // either way, if at some point we woke up more threads than the maximum desired, this will move the active threads
        // to the front of the list, because of the way nsTaskSystem::WakeUpThreads() works
        nsTaskSystem::WakeUpThreads(m_WorkerType, 1);

        WaitForWork();
      }
    }
  }

  return 0;
}

void nsTaskWorkerThread::WaitForWork()
{
  // m_bIsIdle usually will be true here, but may also already have been reset to false
  // in that case m_WakeUpSignal will be raised already and the code below will just run through and continue

  m_ThreadActiveTime += nsTime::Now() - m_StartedWorkingTime;
  m_bExecutingTask = false;
  m_WakeUpSignal.WaitForSignal();
  NS_ASSERT_DEBUG(m_iWorkerState == (int)nsTaskWorkerState::Active, "Worker state should have been reset to 'active'");
}

nsTaskWorkerState nsTaskWorkerThread::WakeUpIfIdle()
{
  nsTaskWorkerState prev = (nsTaskWorkerState)m_iWorkerState.CompareAndSwap((int)nsTaskWorkerState::Idle, (int)nsTaskWorkerState::Active);
  if (prev == nsTaskWorkerState::Idle) // was idle before
  {
    m_WakeUpSignal.RaiseSignal();
  }

  return static_cast<nsTaskWorkerState>(prev);
}

void nsTaskWorkerThread::UpdateThreadUtilization(nsTime timePassed)
{
  nsTime tActive = m_ThreadActiveTime;

  // The thread keeps track of how much time it spends executing tasks.
  // Here we retrieve that time and resets it to zero.
  {
    m_ThreadActiveTime = nsTime::MakeZero();

    if (m_bExecutingTask)
    {
      const nsTime tNow = nsTime::Now();
      tActive += tNow - m_StartedWorkingTime;
      m_StartedWorkingTime = tNow;
    }
  }

  m_fLastThreadUtilization = tActive.GetSeconds() / timePassed.GetSeconds();
  m_uiLastNumTasksExecuted = m_uiNumTasksExecuted;
  m_uiNumTasksExecuted = 0;
}

double nsTaskWorkerThread::GetThreadUtilization(nsUInt32* pNumTasksExecuted /*= nullptr*/)
{
  if (pNumTasksExecuted)
  {
    *pNumTasksExecuted = m_uiLastNumTasksExecuted;
  }

  return m_fLastThreadUtilization;
}
