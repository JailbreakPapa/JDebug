#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/System/SystemInformation.h>
#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/Implementation/TaskWorkerThread.h>
#include <Foundation/Threading/TaskSystem.h>

wdUInt32 wdTaskSystem::GetWorkerThreadCount(wdWorkerThreadType::Enum type)
{
  return s_pThreadState->m_uiMaxWorkersToUse[type];
}

wdUInt32 wdTaskSystem::GetNumAllocatedWorkerThreads(wdWorkerThreadType::Enum type)
{
  return s_pThreadState->m_iAllocatedWorkers[type];
}

void wdTaskSystem::SetWorkerThreadCount(wdInt32 iShortTasks, wdInt32 iLongTasks)
{
  wdSystemInformation info = wdSystemInformation::Get();

  // these settings are supposed to be a sensible default for most applications
  // an app can of course change that to optimize for its own usage
  //
  const wdInt32 iCpuCores = info.GetCPUCoreCount();

  // at least 2 threads, 4 on six cores, 6 on eight cores and up
  if (iShortTasks <= 0)
    iShortTasks = wdMath::Clamp<wdInt32>(iCpuCores - 2, 2, 8);

  // at least 2 threads, 4 on six cores, 6 on eight cores and up
  if (iLongTasks <= 0)
    iLongTasks = wdMath::Clamp<wdInt32>(iCpuCores - 2, 2, 8);

  // plus there is always one additional 'file access' thread
  // and the main thread, of course

  wdUInt32 uiShortTasks = static_cast<wdUInt32>(wdMath::Max<wdInt32>(iShortTasks, 1));
  wdUInt32 uiLongTasks = static_cast<wdUInt32>(wdMath::Max<wdInt32>(iLongTasks, 1));

  // if nothing has changed, do nothing
  if (s_pThreadState->m_uiMaxWorkersToUse[wdWorkerThreadType::ShortTasks] == uiShortTasks &&
      s_pThreadState->m_uiMaxWorkersToUse[wdWorkerThreadType::LongTasks] == uiLongTasks)
    return;

  StopWorkerThreads();

  // this only allocates pointers, i.e. the maximum possible number of threads that we may be able to realloc at runtime
  s_pThreadState->m_Workers[wdWorkerThreadType::ShortTasks].SetCount(1024);
  s_pThreadState->m_Workers[wdWorkerThreadType::LongTasks].SetCount(1024);
  s_pThreadState->m_Workers[wdWorkerThreadType::FileAccess].SetCount(128);

  s_pThreadState->m_uiMaxWorkersToUse[wdWorkerThreadType::ShortTasks] = uiShortTasks;
  s_pThreadState->m_uiMaxWorkersToUse[wdWorkerThreadType::LongTasks] = uiLongTasks;
  s_pThreadState->m_uiMaxWorkersToUse[wdWorkerThreadType::FileAccess] = 1;

  AllocateThreads(wdWorkerThreadType::ShortTasks, s_pThreadState->m_uiMaxWorkersToUse[wdWorkerThreadType::ShortTasks]);
  AllocateThreads(wdWorkerThreadType::LongTasks, s_pThreadState->m_uiMaxWorkersToUse[wdWorkerThreadType::LongTasks]);
  AllocateThreads(wdWorkerThreadType::FileAccess, s_pThreadState->m_uiMaxWorkersToUse[wdWorkerThreadType::FileAccess]);
}

void wdTaskSystem::StopWorkerThreads()
{
  bool bWorkersStillRunning = true;

  // as long as any worker thread is still active, send the wake up signal
  while (bWorkersStillRunning)
  {
    bWorkersStillRunning = false;

    for (wdUInt32 type = 0; type < wdWorkerThreadType::ENUM_COUNT; ++type)
    {
      const wdUInt32 uiNumThreads = s_pThreadState->m_iAllocatedWorkers[type];

      for (wdUInt32 i = 0; i < uiNumThreads; ++i)
      {
        if (s_pThreadState->m_Workers[type][i]->DeactivateWorker().Failed())
        {
          bWorkersStillRunning = true;
        }
      }
    }

    // waste some time
    wdThreadUtils::YieldTimeSlice();
  }

  for (wdUInt32 type = 0; type < wdWorkerThreadType::ENUM_COUNT; ++type)
  {
    const wdUInt32 uiNumWorkers = s_pThreadState->m_iAllocatedWorkers[type];

    for (wdUInt32 i = 0; i < uiNumWorkers; ++i)
    {
      s_pThreadState->m_Workers[type][i]->Join();
      WD_DEFAULT_DELETE(s_pThreadState->m_Workers[type][i]);
    }

    s_pThreadState->m_iAllocatedWorkers[type] = 0;
    s_pThreadState->m_uiMaxWorkersToUse[type] = 0;
    s_pThreadState->m_Workers[type].Clear();
  }
}

void wdTaskSystem::AllocateThreads(wdWorkerThreadType::Enum type, wdUInt32 uiAddThreads)
{
  WD_ASSERT_DEBUG(uiAddThreads > 0, "Invalid number of threads to allocate");

  {
    // prevent concurrent thread allocation
    WD_LOCK(s_TaskSystemMutex);

    wdUInt32 uiNextThreadIdx = s_pThreadState->m_iAllocatedWorkers[type];

    WD_ASSERT_ALWAYS(uiNextThreadIdx + uiAddThreads <= s_pThreadState->m_Workers[type].GetCount(), "Max number of worker threads ({}) exceeded.",
      s_pThreadState->m_Workers[type].GetCount());

    for (wdUInt32 i = 0; i < uiAddThreads; ++i)
    {
      s_pThreadState->m_Workers[type][uiNextThreadIdx] = WD_DEFAULT_NEW(wdTaskWorkerThread, (wdWorkerThreadType::Enum)type, uiNextThreadIdx);
      s_pThreadState->m_Workers[type][uiNextThreadIdx]->Start();

      ++uiNextThreadIdx;
    }

    // let others access the new threads now
    s_pThreadState->m_iAllocatedWorkers[type] = uiNextThreadIdx;
  }

  wdLog::Dev("Allocated {} additional '{}' worker threads ({} total)", uiAddThreads, wdWorkerThreadType::GetThreadTypeName(type),
    s_pThreadState->m_iAllocatedWorkers[type]);
}

void wdTaskSystem::WakeUpThreads(wdWorkerThreadType::Enum type, wdUInt32 uiNumThreadsToWakeUp)
{
  // together with wdTaskWorkerThread::Run() this function will make sure to keep the number
  // of active threads close to m_uiMaxWorkersToUse
  //
  // threads that go into the 'blocked' state will raise the number of threads that get activated
  // and when they are unblocked, together they may exceed the 'maximum' number of active threads
  // but over time the threads at the end of the list will put themselves to sleep again

  auto* s = wdTaskSystem::s_pThreadState.Borrow();

  const wdUInt32 uiTotalThreads = s_pThreadState->m_iAllocatedWorkers[type];
  wdUInt32 uiAllowedActiveThreads = s_pThreadState->m_uiMaxWorkersToUse[type];

  for (wdUInt32 threadIdx = 0; threadIdx < uiTotalThreads; ++threadIdx)
  {
    switch (s->m_Workers[type][threadIdx]->WakeUpIfIdle())
    {
      case wdTaskWorkerState::Idle:
      {
        // was idle before -> now it is active
        if (--uiNumThreadsToWakeUp == 0)
          return;

        [[fallthrough]];
      }

      case wdTaskWorkerState::Active:
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
    AllocateThreads(type, wdMath::Min(uiNumThreadsToWakeUp, uiAllowedActiveThreads));
  }
}

wdWorkerThreadType::Enum wdTaskSystem::GetCurrentThreadWorkerType()
{
  return tl_TaskWorkerInfo.m_WorkerType;
}

double wdTaskSystem::GetThreadUtilization(wdWorkerThreadType::Enum type, wdUInt32 uiThreadIndex, wdUInt32* pNumTasksExecuted /*= nullptr*/)
{
  return s_pThreadState->m_Workers[type][uiThreadIndex]->GetThreadUtilization(pNumTasksExecuted);
}

void wdTaskSystem::DetermineTasksToExecuteOnThread(wdTaskPriority::Enum& out_FirstPriority, wdTaskPriority::Enum& out_LastPriority)
{
  switch (tl_TaskWorkerInfo.m_WorkerType)
  {
    case wdWorkerThreadType::MainThread:
    {
      out_FirstPriority = wdTaskPriority::ThisFrameMainThread;
      out_LastPriority = wdTaskPriority::SomeFrameMainThread;
      break;
    }

    case wdWorkerThreadType::FileAccess:
    {
      out_FirstPriority = wdTaskPriority::FileAccessHighPriority;
      out_LastPriority = wdTaskPriority::FileAccess;
      break;
    }

    case wdWorkerThreadType::LongTasks:
    {
      out_FirstPriority = wdTaskPriority::LongRunningHighPriority;
      out_LastPriority = wdTaskPriority::LongRunning;
      break;
    }

    case wdWorkerThreadType::ShortTasks:
    {
      out_FirstPriority = wdTaskPriority::EarlyThisFrame;
      out_LastPriority = wdTaskPriority::In9Frames;
      break;
    }

    case wdWorkerThreadType::Unknown:
    {
      // probably a thread not launched through wd
      out_FirstPriority = wdTaskPriority::EarlyThisFrame;
      out_LastPriority = wdTaskPriority::In9Frames;
      break;
    }

    default:
    {
      WD_ASSERT_NOT_IMPLEMENTED;
      break;
    }
  }
}


WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_TaskSystemThreads);
