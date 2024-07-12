#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/Implementation/TaskWorkerThread.h>
#include <Foundation/Threading/TaskSystem.h>

nsUInt32 nsTaskSystem::GetWorkerThreadCount(nsWorkerThreadType::Enum type)
{
  return s_pThreadState->m_uiMaxWorkersToUse[type];
}

nsUInt32 nsTaskSystem::GetNumAllocatedWorkerThreads(nsWorkerThreadType::Enum type)
{
  return s_pThreadState->m_iAllocatedWorkers[type];
}

void nsTaskSystem::SetWorkerThreadCount(nsInt32 iShortTasks, nsInt32 iLongTasks)
{
  nsSystemInformation info = nsSystemInformation::Get();

  // these settings are supposed to be a sensible default for most applications
  // an app can of course change that to optimize for its own usage
  //
  const nsInt32 iCpuCores = info.GetCPUCoreCount();

  // at least 2 threads, 4 on six cores, 6 on eight cores and up
  if (iShortTasks <= 0)
    iShortTasks = nsMath::Clamp<nsInt32>(iCpuCores - 2, 2, 8);

  // at least 2 threads, 4 on six cores, 6 on eight cores and up
  if (iLongTasks <= 0)
    iLongTasks = nsMath::Clamp<nsInt32>(iCpuCores - 2, 2, 8);

  // plus there is always one additional 'file access' thread
  // and the main thread, of course

  nsUInt32 uiShortTasks = static_cast<nsUInt32>(nsMath::Max<nsInt32>(iShortTasks, 1));
  nsUInt32 uiLongTasks = static_cast<nsUInt32>(nsMath::Max<nsInt32>(iLongTasks, 1));

  // if nothing has changed, do nothing
  if (s_pThreadState->m_uiMaxWorkersToUse[nsWorkerThreadType::ShortTasks] == uiShortTasks &&
      s_pThreadState->m_uiMaxWorkersToUse[nsWorkerThreadType::LongTasks] == uiLongTasks)
    return;

  StopWorkerThreads();

  // this only allocates pointers, i.e. the maximum possible number of threads that we may be able to realloc at runtime
  s_pThreadState->m_Workers[nsWorkerThreadType::ShortTasks].SetCount(1024);
  s_pThreadState->m_Workers[nsWorkerThreadType::LongTasks].SetCount(1024);
  s_pThreadState->m_Workers[nsWorkerThreadType::FileAccess].SetCount(128);

  s_pThreadState->m_uiMaxWorkersToUse[nsWorkerThreadType::ShortTasks] = uiShortTasks;
  s_pThreadState->m_uiMaxWorkersToUse[nsWorkerThreadType::LongTasks] = uiLongTasks;
  s_pThreadState->m_uiMaxWorkersToUse[nsWorkerThreadType::FileAccess] = 1;

  AllocateThreads(nsWorkerThreadType::ShortTasks, s_pThreadState->m_uiMaxWorkersToUse[nsWorkerThreadType::ShortTasks]);
  AllocateThreads(nsWorkerThreadType::LongTasks, s_pThreadState->m_uiMaxWorkersToUse[nsWorkerThreadType::LongTasks]);
  AllocateThreads(nsWorkerThreadType::FileAccess, s_pThreadState->m_uiMaxWorkersToUse[nsWorkerThreadType::FileAccess]);
}

void nsTaskSystem::StopWorkerThreads()
{
  bool bWorkersStillRunning = true;

  // as long as any worker thread is still active, send the wake up signal
  while (bWorkersStillRunning)
  {
    bWorkersStillRunning = false;

    for (nsUInt32 type = 0; type < nsWorkerThreadType::ENUM_COUNT; ++type)
    {
      const nsUInt32 uiNumThreads = s_pThreadState->m_iAllocatedWorkers[type];

      for (nsUInt32 i = 0; i < uiNumThreads; ++i)
      {
        if (s_pThreadState->m_Workers[type][i]->DeactivateWorker().Failed())
        {
          bWorkersStillRunning = true;
        }
      }
    }

    // waste some time
    nsThreadUtils::YieldTimeSlice();
  }

  for (nsUInt32 type = 0; type < nsWorkerThreadType::ENUM_COUNT; ++type)
  {
    const nsUInt32 uiNumWorkers = s_pThreadState->m_iAllocatedWorkers[type];

    for (nsUInt32 i = 0; i < uiNumWorkers; ++i)
    {
      s_pThreadState->m_Workers[type][i]->Join();
      NS_DEFAULT_DELETE(s_pThreadState->m_Workers[type][i]);
    }

    s_pThreadState->m_iAllocatedWorkers[type] = 0;
    s_pThreadState->m_uiMaxWorkersToUse[type] = 0;
    s_pThreadState->m_Workers[type].Clear();
  }
}

void nsTaskSystem::AllocateThreads(nsWorkerThreadType::Enum type, nsUInt32 uiAddThreads)
{
  NS_ASSERT_DEBUG(uiAddThreads > 0, "Invalid number of threads to allocate");

  {
    // prevent concurrent thread allocation
    NS_LOCK(s_TaskSystemMutex);

    nsUInt32 uiNextThreadIdx = s_pThreadState->m_iAllocatedWorkers[type];

    NS_ASSERT_ALWAYS(uiNextThreadIdx + uiAddThreads <= s_pThreadState->m_Workers[type].GetCount(), "Max number of worker threads ({}) exceeded.",
      s_pThreadState->m_Workers[type].GetCount());

    for (nsUInt32 i = 0; i < uiAddThreads; ++i)
    {
      s_pThreadState->m_Workers[type][uiNextThreadIdx] = NS_DEFAULT_NEW(nsTaskWorkerThread, (nsWorkerThreadType::Enum)type, uiNextThreadIdx);
      s_pThreadState->m_Workers[type][uiNextThreadIdx]->Start();

      ++uiNextThreadIdx;
    }

    // let others access the new threads now
    s_pThreadState->m_iAllocatedWorkers[type] = uiNextThreadIdx;
  }

  nsLog::Dev("Allocated {} additional '{}' worker threads ({} total)", uiAddThreads, nsWorkerThreadType::GetThreadTypeName(type),
    s_pThreadState->m_iAllocatedWorkers[type]);
}

void nsTaskSystem::WakeUpThreads(nsWorkerThreadType::Enum type, nsUInt32 uiNumThreadsToWakeUp)
{
  // together with nsTaskWorkerThread::Run() this function will make sure to keep the number
  // of active threads close to m_uiMaxWorkersToUse
  //
  // threads that go into the 'blocked' state will raise the number of threads that get activated
  // and when they are unblocked, together they may exceed the 'maximum' number of active threads
  // but over time the threads at the end of the list will put themselves to sleep again

  auto* s = nsTaskSystem::s_pThreadState.Borrow();

  const nsUInt32 uiTotalThreads = s_pThreadState->m_iAllocatedWorkers[type];
  nsUInt32 uiAllowedActiveThreads = s_pThreadState->m_uiMaxWorkersToUse[type];

  for (nsUInt32 threadIdx = 0; threadIdx < uiTotalThreads; ++threadIdx)
  {
    switch (s->m_Workers[type][threadIdx]->WakeUpIfIdle())
    {
      case nsTaskWorkerState::Idle:
      {
        // was idle before -> now it is active
        if (--uiNumThreadsToWakeUp == 0)
          return;

        [[fallthrough]];
      }

      case nsTaskWorkerState::Active:
      {
        // already active
        if (--uiAllowedActiveThreads == 0)
          return;

        break;
      }

      default:
        break;
    }
  }

  // if the loop above did not find enough threads to wake up
  if (uiNumThreadsToWakeUp > 0 && uiAllowedActiveThreads > 0)
  {
    // the new threads will start not-idle and take on some work
    AllocateThreads(type, nsMath::Min(uiNumThreadsToWakeUp, uiAllowedActiveThreads));
  }
}

nsWorkerThreadType::Enum nsTaskSystem::GetCurrentThreadWorkerType()
{
  return tl_TaskWorkerInfo.m_WorkerType;
}

double nsTaskSystem::GetThreadUtilization(nsWorkerThreadType::Enum type, nsUInt32 uiThreadIndex, nsUInt32* pNumTasksExecuted /*= nullptr*/)
{
  return s_pThreadState->m_Workers[type][uiThreadIndex]->GetThreadUtilization(pNumTasksExecuted);
}

void nsTaskSystem::DetermineTasksToExecuteOnThread(nsTaskPriority::Enum& out_FirstPriority, nsTaskPriority::Enum& out_LastPriority)
{
  switch (tl_TaskWorkerInfo.m_WorkerType)
  {
    case nsWorkerThreadType::MainThread:
    {
      out_FirstPriority = nsTaskPriority::ThisFrameMainThread;
      out_LastPriority = nsTaskPriority::SomeFrameMainThread;
      break;
    }

    case nsWorkerThreadType::FileAccess:
    {
      out_FirstPriority = nsTaskPriority::FileAccessHighPriority;
      out_LastPriority = nsTaskPriority::FileAccess;
      break;
    }

    case nsWorkerThreadType::LongTasks:
    {
      out_FirstPriority = nsTaskPriority::LongRunningHighPriority;
      out_LastPriority = nsTaskPriority::LongRunning;
      break;
    }

    case nsWorkerThreadType::ShortTasks:
    {
      out_FirstPriority = nsTaskPriority::EarlyThisFrame;
      out_LastPriority = nsTaskPriority::In9Frames;
      break;
    }

    case nsWorkerThreadType::Unknown:
    {
      // probably a thread not launched through ns
      out_FirstPriority = nsTaskPriority::EarlyThisFrame;
      out_LastPriority = nsTaskPriority::In9Frames;
      break;
    }

    default:
    {
      NS_ASSERT_NOT_IMPLEMENTED;
      break;
    }
  }
}
