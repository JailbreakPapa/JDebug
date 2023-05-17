#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/Implementation/TaskWorkerThread.h>
#include <Foundation/Threading/TaskSystem.h>

thread_local wdTaskWorkerInfo tl_TaskWorkerInfo;

static const char* GenerateThreadName(wdWorkerThreadType::Enum threadType, wdUInt32 uiThreadNumber)
{
  static wdStringBuilder sTemp;
  sTemp.Format("{} {}", wdWorkerThreadType::GetThreadTypeName(threadType), uiThreadNumber);
  return sTemp.GetData();
}

wdTaskWorkerThread::wdTaskWorkerThread(wdWorkerThreadType::Enum threadType, wdUInt32 uiThreadNumber)
  // We need at least 256 kb of stack size, otherwise the shader compilation tasks will run out of stack space.
  : wdThread(GenerateThreadName(threadType, uiThreadNumber), 256 * 1024)
{
  m_WorkerType = threadType;
  m_uiWorkerThreadNumber = uiThreadNumber & 0xFFFF;
}

wdTaskWorkerThread::~wdTaskWorkerThread() = default;

wdResult wdTaskWorkerThread::DeactivateWorker()
{
  m_bActive = false;

  if (GetThreadStatus() != wdThread::Finished)
  {
    // if necessary, wake this thread up
    WakeUpIfIdle();

    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

wdUInt32 wdTaskWorkerThread::Run()
{
  WD_ASSERT_DEBUG(
    m_WorkerType != wdWorkerThreadType::Unknown && m_WorkerType != wdWorkerThreadType::MainThread, "Worker threads cannot use this type");
  WD_ASSERT_DEBUG(m_WorkerType < wdWorkerThreadType::ENUM_COUNT, "Worker Thread Type is invalid: {0}", m_WorkerType);

  // once this thread is running, store the worker type in the thread_local variable
  // such that the wdTaskSystem is able to look this up (e.g. in WaitForGroup) to know which types of tasks to help with
  tl_TaskWorkerInfo.m_WorkerType = m_WorkerType;
  tl_TaskWorkerInfo.m_iWorkerIndex = m_uiWorkerThreadNumber;
  tl_TaskWorkerInfo.m_pWorkerState = &m_iWorkerState;

  const bool bIsReserve = m_uiWorkerThreadNumber >= wdTaskSystem::s_pThreadState->m_uiMaxWorkersToUse[m_WorkerType];

  wdTaskPriority::Enum FirstPriority;
  wdTaskPriority::Enum LastPriority;
  wdTaskSystem::DetermineTasksToExecuteOnThread(FirstPriority, LastPriority);

  m_bExecutingTask = false;

  while (m_bActive)
  {
    if (!m_bExecutingTask)
    {
      m_bExecutingTask = true;
      m_StartedWorkingTime = wdTime::Now();
    }

    if (!wdTaskSystem::ExecuteTask(FirstPriority, LastPriority, false, wdTaskGroupID(), &m_iWorkerState))
    {
      WaitForWork();
    }
    else
    {
      ++m_uiNumTasksExecuted;

      if (bIsReserve)
      {
        WD_VERIFY(m_iWorkerState.Set((int)wdTaskWorkerState::Idle) == (int)wdTaskWorkerState::Active, "Corrupt worker state");

        // if this thread is part of the reserve, then don't continue to process tasks indefinitely
        // instead, put this thread to sleep and wake up someone else
        // that someone else may be a thread at the front of the queue, it may also turn out to be this thread again
        // either way, if at some point we woke up more threads than the maximum desired, this will move the active threads
        // to the front of the list, because of the way wdTaskSystem::WakeUpThreads() works
        wdTaskSystem::WakeUpThreads(m_WorkerType, 1);

        WaitForWork();
      }
    }
  }

  return 0;
}

void wdTaskWorkerThread::WaitForWork()
{
  // m_bIsIdle usually will be true here, but may also already have been reset to false
  // in that case m_WakeUpSignal will be raised already and the code below will just run through and continue

  m_ThreadActiveTime += wdTime::Now() - m_StartedWorkingTime;
  m_bExecutingTask = false;
  m_WakeUpSignal.WaitForSignal();
  WD_ASSERT_DEBUG(m_iWorkerState == (int)wdTaskWorkerState::Active, "Worker state should have been reset to 'active'");
}

wdTaskWorkerState wdTaskWorkerThread::WakeUpIfIdle()
{
  wdTaskWorkerState prev = (wdTaskWorkerState)m_iWorkerState.CompareAndSwap((int)wdTaskWorkerState::Idle, (int)wdTaskWorkerState::Active);
  if (prev == wdTaskWorkerState::Idle) // was idle before
  {
    m_WakeUpSignal.RaiseSignal();
  }

  return static_cast<wdTaskWorkerState>(prev);
}

void wdTaskWorkerThread::UpdateThreadUtilization(wdTime timePassed)
{
  wdTime tActive = m_ThreadActiveTime;

  // The thread keeps track of how much time it spends executing tasks.
  // Here we retrieve that time and resets it to zero.
  {
    m_ThreadActiveTime = wdTime::Zero();

    if (m_bExecutingTask)
    {
      const wdTime tNow = wdTime::Now();
      tActive += tNow - m_StartedWorkingTime;
      m_StartedWorkingTime = tNow;
    }
  }

  m_fLastThreadUtilization = tActive.GetSeconds() / timePassed.GetSeconds();
  m_uiLastNumTasksExecuted = m_uiNumTasksExecuted;
  m_uiNumTasksExecuted = 0;
}

double wdTaskWorkerThread::GetThreadUtilization(wdUInt32* pNumTasksExecuted /*= nullptr*/)
{
  if (pNumTasksExecuted)
  {
    *pNumTasksExecuted = m_uiLastNumTasksExecuted;
  }

  return m_fLastThreadUtilization;
}


WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_TaskWorkerThread);
