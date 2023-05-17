#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/Implementation/TaskGroup.h>
#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/Implementation/TaskWorkerThread.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/TaskSystem.h>


wdTaskGroupID wdTaskSystem::CreateTaskGroup(wdTaskPriority::Enum priority, wdOnTaskGroupFinishedCallback callback)
{
  WD_LOCK(s_TaskSystemMutex);

  wdUInt32 i = 0;

  // this search could be speed up with a stack of free groups
  for (; i < s_pState->m_TaskGroups.GetCount(); ++i)
  {
    if (!s_pState->m_TaskGroups[i].m_bInUse)
    {
      goto foundtaskgroup;
    }
  }

  // no free group found, create a new one
  s_pState->m_TaskGroups.ExpandAndGetRef();
  s_pState->m_TaskGroups[i].m_uiTaskGroupIndex = static_cast<wdUInt16>(i);

foundtaskgroup:

  s_pState->m_TaskGroups[i].Reuse(priority, callback);

  wdTaskGroupID id;
  id.m_pTaskGroup = &s_pState->m_TaskGroups[i];
  id.m_uiGroupCounter = s_pState->m_TaskGroups[i].m_uiGroupCounter;
  return id;
}

void wdTaskSystem::AddTaskToGroup(wdTaskGroupID groupID, const wdSharedPtr<wdTask>& pTask)
{
  WD_ASSERT_DEBUG(pTask != nullptr, "Cannot add nullptr tasks.");
  WD_ASSERT_DEV(pTask->IsTaskFinished(), "The given task is not finished! Cannot reuse a task before it is done.");
  WD_ASSERT_DEBUG(!pTask->m_sTaskName.IsEmpty(), "Every task should have a name");

  wdTaskGroup::DebugCheckTaskGroup(groupID, s_TaskSystemMutex);

  pTask->Reset();
  pTask->m_BelongsToGroup = groupID;
  groupID.m_pTaskGroup->m_Tasks.PushBack(pTask);
}

void wdTaskSystem::AddTaskGroupDependency(wdTaskGroupID groupID, wdTaskGroupID dependsOn)
{
  WD_ASSERT_DEBUG(dependsOn.IsValid(), "Invalid dependency");
  WD_ASSERT_DEBUG(groupID.m_pTaskGroup != dependsOn.m_pTaskGroup || groupID.m_uiGroupCounter != dependsOn.m_uiGroupCounter, "Group cannot depend on itselfs");

  wdTaskGroup::DebugCheckTaskGroup(groupID, s_TaskSystemMutex);

  groupID.m_pTaskGroup->m_DependsOnGroups.PushBack(dependsOn);
}

void wdTaskSystem::AddTaskGroupDependencyBatch(wdArrayPtr<const wdTaskGroupDependency> batch)
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  // lock here once to reduce the overhead of wdTaskGroup::DebugCheckTaskGroup inside AddTaskGroupDependency
  WD_LOCK(s_TaskSystemMutex);
#endif

  for (const wdTaskGroupDependency& dep : batch)
  {
    AddTaskGroupDependency(dep.m_TaskGroup, dep.m_DependsOn);
  }
}

void wdTaskSystem::StartTaskGroup(wdTaskGroupID groupID)
{
  WD_ASSERT_DEV(s_pThreadState->m_Workers[wdWorkerThreadType::ShortTasks].GetCount() > 0, "No worker threads started.");

  wdTaskGroup::DebugCheckTaskGroup(groupID, s_TaskSystemMutex);

  wdInt32 iActiveDependencies = 0;

  {
    WD_LOCK(s_TaskSystemMutex);

    wdTaskGroup& tg = *groupID.m_pTaskGroup;

    tg.m_bStartedByUser = true;

    for (wdUInt32 i = 0; i < tg.m_DependsOnGroups.GetCount(); ++i)
    {
      if (!IsTaskGroupFinished(tg.m_DependsOnGroups[i]))
      {
        wdTaskGroup& Dependency = *tg.m_DependsOnGroups[i].m_pTaskGroup;

        // add this task group to the list of dependencies, such that when that group finishes, this task group can get woken up
        Dependency.m_OthersDependingOnMe.PushBack(groupID);

        // count how many other groups need to finish before this task group can be executed
        ++iActiveDependencies;
      }
    }

    if (iActiveDependencies != 0)
    {
      // atomic integers are quite slow, so do not use them in the loop, where they are not yet needed
      tg.m_iNumActiveDependencies = iActiveDependencies;
    }
  }

  if (iActiveDependencies == 0)
  {
    ScheduleGroupTasks(groupID.m_pTaskGroup, false);
  }
}

void wdTaskSystem::StartTaskGroupBatch(wdArrayPtr<const wdTaskGroupID> batch)
{
  WD_LOCK(s_TaskSystemMutex);

  for (const wdTaskGroupID& group : batch)
  {
    StartTaskGroup(group);
  }
}

bool wdTaskSystem::IsTaskGroupFinished(wdTaskGroupID group)
{
  // if the counters differ, the task group has been reused since the GroupID was created, so that group has finished
  return (group.m_pTaskGroup == nullptr) || (group.m_pTaskGroup->m_uiGroupCounter != group.m_uiGroupCounter);
}

void wdTaskSystem::ScheduleGroupTasks(wdTaskGroup* pGroup, bool bHighPriority)
{
  if (pGroup->m_Tasks.IsEmpty())
  {
    pGroup->m_iNumRemainingTasks = 1;

    // "finish" one task -> will finish the task group and kick off dependent groups
    TaskHasFinished(nullptr, pGroup);
    return;
  }

  wdInt32 iRemainingTasks = 0;

  // add all the tasks to the task list, so that they will be processed
  {
    WD_LOCK(s_TaskSystemMutex);


    // store how many tasks from this groups still need to be processed

    for (auto pTask : pGroup->m_Tasks)
    {
      iRemainingTasks += wdMath::Max(1u, pTask->m_uiMultiplicity);
      pTask->m_iRemainingRuns = wdMath::Max(1u, pTask->m_uiMultiplicity);
    }

    pGroup->m_iNumRemainingTasks = iRemainingTasks;


    for (wdUInt32 task = 0; task < pGroup->m_Tasks.GetCount(); ++task)
    {
      auto& pTask = pGroup->m_Tasks[task];

      for (wdUInt32 mult = 0; mult < wdMath::Max(1u, pTask->m_uiMultiplicity); ++mult)
      {
        TaskData td;
        td.m_pBelongsToGroup = pGroup;
        td.m_pTask = pTask;
        td.m_pTask->m_bTaskIsScheduled = true;
        td.m_uiInvocation = mult;

        if (bHighPriority)
          s_pState->m_Tasks[pGroup->m_Priority].PushFront(td);
        else
          s_pState->m_Tasks[pGroup->m_Priority].PushBack(td);
      }
    }

    // send the proper thread signal, to make sure one of the correct worker threads is awake
    switch (pGroup->m_Priority)
    {
      case wdTaskPriority::EarlyThisFrame:
      case wdTaskPriority::ThisFrame:
      case wdTaskPriority::LateThisFrame:
      case wdTaskPriority::EarlyNextFrame:
      case wdTaskPriority::NextFrame:
      case wdTaskPriority::LateNextFrame:
      case wdTaskPriority::In2Frames:
      case wdTaskPriority::In3Frames:
      case wdTaskPriority::In4Frames:
      case wdTaskPriority::In5Frames:
      case wdTaskPriority::In6Frames:
      case wdTaskPriority::In7Frames:
      case wdTaskPriority::In8Frames:
      case wdTaskPriority::In9Frames:
      {
        WakeUpThreads(wdWorkerThreadType::ShortTasks, iRemainingTasks);
        break;
      }

      case wdTaskPriority::LongRunning:
      case wdTaskPriority::LongRunningHighPriority:
      {
        WakeUpThreads(wdWorkerThreadType::LongTasks, iRemainingTasks);
        break;
      }

      case wdTaskPriority::FileAccess:
      case wdTaskPriority::FileAccessHighPriority:
      {
        WakeUpThreads(wdWorkerThreadType::FileAccess, iRemainingTasks);
        break;
      }

      case wdTaskPriority::SomeFrameMainThread:
      case wdTaskPriority::ThisFrameMainThread:
      case wdTaskPriority::ENUM_COUNT:
        // nothing to do for these enum values
        break;
    }
  }
}

void wdTaskSystem::DependencyHasFinished(wdTaskGroup* pGroup)
{
  // remove one dependency from the group
  if (pGroup->m_iNumActiveDependencies.Decrement() == 0)
  {
    // if there are no remaining dependencies, kick off all tasks in this group
    ScheduleGroupTasks(pGroup, true);
  }
}

wdResult wdTaskSystem::CancelGroup(wdTaskGroupID group, wdOnTaskRunning::Enum onTaskRunning)
{
  if (wdTaskSystem::IsTaskGroupFinished(group))
    return WD_SUCCESS;

  WD_PROFILE_SCOPE("CancelGroup");

  WD_LOCK(s_TaskSystemMutex);

  wdResult res = WD_SUCCESS;

  auto TasksCopy = group.m_pTaskGroup->m_Tasks;

  // first cancel ALL the tasks in the group, without waiting for anything
  for (wdUInt32 task = 0; task < TasksCopy.GetCount(); ++task)
  {
    if (CancelTask(TasksCopy[task], wdOnTaskRunning::ReturnWithoutBlocking) == WD_FAILURE)
    {
      res = WD_FAILURE;
    }
  }

  // if all tasks could be removed without problems, we do not need to try it again with blocking

  if (onTaskRunning == wdOnTaskRunning::WaitTillFinished && res == WD_FAILURE)
  {
    // now cancel the tasks in the group again, this time wait for those that are already running
    for (wdUInt32 task = 0; task < TasksCopy.GetCount(); ++task)
    {
      CancelTask(TasksCopy[task], wdOnTaskRunning::WaitTillFinished).IgnoreResult();
    }
  }

  return res;
}

void wdTaskSystem::WaitForGroup(wdTaskGroupID group)
{
  WD_PROFILE_SCOPE("WaitForGroup");

  WD_ASSERT_DEV(tl_TaskWorkerInfo.m_bAllowNestedTasks, "The executing task '{}' is flagged to never wait for other tasks but does so anyway. Remove the flag or remove the wait-dependency.", tl_TaskWorkerInfo.m_szTaskName);

  const auto ThreadTaskType = tl_TaskWorkerInfo.m_WorkerType;
  const bool bAllowSleep = ThreadTaskType != wdWorkerThreadType::MainThread;

  while (!wdTaskSystem::IsTaskGroupFinished(group))
  {
    if (!HelpExecutingTasks(group))
    {
      if (bAllowSleep)
      {
        const wdWorkerThreadType::Enum typeToWakeUp = (ThreadTaskType == wdWorkerThreadType::Unknown) ? wdWorkerThreadType::ShortTasks : ThreadTaskType;

        if (tl_TaskWorkerInfo.m_pWorkerState)
        {
          WD_VERIFY(tl_TaskWorkerInfo.m_pWorkerState->Set((int)wdTaskWorkerState::Blocked) == (int)wdTaskWorkerState::Active, "Corrupt worker state");
        }

        WakeUpThreads(typeToWakeUp, 1);

        group.m_pTaskGroup->WaitForFinish(group);

        if (tl_TaskWorkerInfo.m_pWorkerState)
        {
          WD_VERIFY(tl_TaskWorkerInfo.m_pWorkerState->Set((int)wdTaskWorkerState::Active) == (int)wdTaskWorkerState::Blocked, "Corrupt worker state");
        }

        break;
      }
      else
      {
        wdThreadUtils::YieldTimeSlice();
      }
    }
  }
}

void wdTaskSystem::WaitForCondition(wdDelegate<bool()> condition)
{
  WD_PROFILE_SCOPE("WaitForCondition");

  WD_ASSERT_DEV(tl_TaskWorkerInfo.m_bAllowNestedTasks, "The executing task '{}' is flagged to never wait for other tasks but does so anyway. Remove the flag or remove the wait-dependency.", tl_TaskWorkerInfo.m_szTaskName);

  const auto ThreadTaskType = tl_TaskWorkerInfo.m_WorkerType;
  const bool bAllowSleep = ThreadTaskType != wdWorkerThreadType::MainThread;

  while (!condition())
  {
    if (!HelpExecutingTasks(wdTaskGroupID()))
    {
      if (bAllowSleep)
      {
        const wdWorkerThreadType::Enum typeToWakeUp = (ThreadTaskType == wdWorkerThreadType::Unknown) ? wdWorkerThreadType::ShortTasks : ThreadTaskType;

        if (tl_TaskWorkerInfo.m_pWorkerState)
        {
          WD_VERIFY(tl_TaskWorkerInfo.m_pWorkerState->Set((int)wdTaskWorkerState::Blocked) == (int)wdTaskWorkerState::Active, "Corrupt worker state");
        }

        WakeUpThreads(typeToWakeUp, 1);

        while (!condition())
        {
          // TODO: busy loop for now
          wdThreadUtils::YieldTimeSlice();
        }

        if (tl_TaskWorkerInfo.m_pWorkerState)
        {
          WD_VERIFY(tl_TaskWorkerInfo.m_pWorkerState->Set((int)wdTaskWorkerState::Active) == (int)wdTaskWorkerState::Blocked, "Corrupt worker state");
        }

        break;
      }
      else
      {
        wdThreadUtils::YieldTimeSlice();
      }
    }
  }
}

WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_TaskSystemGroups);
