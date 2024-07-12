#include <Foundation/FoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/Implementation/TaskGroup.h>
#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/Implementation/TaskWorkerThread.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/TaskSystem.h>


nsTaskGroupID nsTaskSystem::CreateTaskGroup(nsTaskPriority::Enum priority, nsOnTaskGroupFinishedCallback callback)
{
  NS_LOCK(s_TaskSystemMutex);

  nsUInt32 i = 0;

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
  s_pState->m_TaskGroups[i].m_uiTaskGroupIndex = static_cast<nsUInt16>(i);

foundtaskgroup:

  s_pState->m_TaskGroups[i].Reuse(priority, callback);

  nsTaskGroupID id;
  id.m_pTaskGroup = &s_pState->m_TaskGroups[i];
  id.m_uiGroupCounter = s_pState->m_TaskGroups[i].m_uiGroupCounter;
  return id;
}

void nsTaskSystem::AddTaskToGroup(nsTaskGroupID groupID, const nsSharedPtr<nsTask>& pTask)
{
  NS_ASSERT_DEBUG(pTask != nullptr, "Cannot add nullptr tasks.");
  NS_ASSERT_DEV(pTask->IsTaskFinished(), "The given task is not finished! Cannot reuse a task before it is done.");
  NS_ASSERT_DEBUG(!pTask->m_sTaskName.IsEmpty(), "Every task should have a name");

  nsTaskGroup::DebugCheckTaskGroup(groupID, s_TaskSystemMutex);

  pTask->Reset();
  pTask->m_BelongsToGroup = groupID;
  groupID.m_pTaskGroup->m_Tasks.PushBack(pTask);
}

void nsTaskSystem::AddTaskGroupDependency(nsTaskGroupID groupID, nsTaskGroupID dependsOn)
{
  NS_ASSERT_DEBUG(dependsOn.IsValid(), "Invalid dependency");
  NS_ASSERT_DEBUG(groupID.m_pTaskGroup != dependsOn.m_pTaskGroup || groupID.m_uiGroupCounter != dependsOn.m_uiGroupCounter, "Group cannot depend on itselfs");

  nsTaskGroup::DebugCheckTaskGroup(groupID, s_TaskSystemMutex);

  groupID.m_pTaskGroup->m_DependsOnGroups.PushBack(dependsOn);
}

void nsTaskSystem::AddTaskGroupDependencyBatch(nsArrayPtr<const nsTaskGroupDependency> batch)
{
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  // lock here once to reduce the overhead of nsTaskGroup::DebugCheckTaskGroup inside AddTaskGroupDependency
  NS_LOCK(s_TaskSystemMutex);
#endif

  for (const nsTaskGroupDependency& dep : batch)
  {
    AddTaskGroupDependency(dep.m_TaskGroup, dep.m_DependsOn);
  }
}

void nsTaskSystem::StartTaskGroup(nsTaskGroupID groupID)
{
  NS_ASSERT_DEV(s_pThreadState->m_Workers[nsWorkerThreadType::ShortTasks].GetCount() > 0, "No worker threads started.");

  nsTaskGroup::DebugCheckTaskGroup(groupID, s_TaskSystemMutex);

  nsInt32 iActiveDependencies = 0;

  {
    NS_LOCK(s_TaskSystemMutex);

    nsTaskGroup& tg = *groupID.m_pTaskGroup;

    tg.m_bStartedByUser = true;

    for (nsUInt32 i = 0; i < tg.m_DependsOnGroups.GetCount(); ++i)
    {
      if (!IsTaskGroupFinished(tg.m_DependsOnGroups[i]))
      {
        nsTaskGroup& Dependency = *tg.m_DependsOnGroups[i].m_pTaskGroup;

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

void nsTaskSystem::StartTaskGroupBatch(nsArrayPtr<const nsTaskGroupID> batch)
{
  NS_LOCK(s_TaskSystemMutex);

  for (const nsTaskGroupID& group : batch)
  {
    StartTaskGroup(group);
  }
}

bool nsTaskSystem::IsTaskGroupFinished(nsTaskGroupID group)
{
  // if the counters differ, the task group has been reused since the GroupID was created, so that group has finished
  return (group.m_pTaskGroup == nullptr) || (group.m_pTaskGroup->m_uiGroupCounter != group.m_uiGroupCounter);
}

void nsTaskSystem::ScheduleGroupTasks(nsTaskGroup* pGroup, bool bHighPriority)
{
  if (pGroup->m_Tasks.IsEmpty())
  {
    pGroup->m_iNumRemainingTasks = 1;

    // "finish" one task -> will finish the task group and kick off dependent groups
    TaskHasFinished(nullptr, pGroup);
    return;
  }

  nsInt32 iRemainingTasks = 0;

  // add all the tasks to the task list, so that they will be processed
  {
    NS_LOCK(s_TaskSystemMutex);


    // store how many tasks from this groups still need to be processed

    for (auto pTask : pGroup->m_Tasks)
    {
      iRemainingTasks += nsMath::Max(1u, pTask->m_uiMultiplicity);
      pTask->m_iRemainingRuns = nsMath::Max(1u, pTask->m_uiMultiplicity);
    }

    pGroup->m_iNumRemainingTasks = iRemainingTasks;


    for (nsUInt32 task = 0; task < pGroup->m_Tasks.GetCount(); ++task)
    {
      auto& pTask = pGroup->m_Tasks[task];

      for (nsUInt32 mult = 0; mult < nsMath::Max(1u, pTask->m_uiMultiplicity); ++mult)
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
      case nsTaskPriority::EarlyThisFrame:
      case nsTaskPriority::ThisFrame:
      case nsTaskPriority::LateThisFrame:
      case nsTaskPriority::EarlyNextFrame:
      case nsTaskPriority::NextFrame:
      case nsTaskPriority::LateNextFrame:
      case nsTaskPriority::In2Frames:
      case nsTaskPriority::In3Frames:
      case nsTaskPriority::In4Frames:
      case nsTaskPriority::In5Frames:
      case nsTaskPriority::In6Frames:
      case nsTaskPriority::In7Frames:
      case nsTaskPriority::In8Frames:
      case nsTaskPriority::In9Frames:
      {
        WakeUpThreads(nsWorkerThreadType::ShortTasks, iRemainingTasks);
        break;
      }

      case nsTaskPriority::LongRunning:
      case nsTaskPriority::LongRunningHighPriority:
      {
        WakeUpThreads(nsWorkerThreadType::LongTasks, iRemainingTasks);
        break;
      }

      case nsTaskPriority::FileAccess:
      case nsTaskPriority::FileAccessHighPriority:
      {
        WakeUpThreads(nsWorkerThreadType::FileAccess, iRemainingTasks);
        break;
      }

      case nsTaskPriority::SomeFrameMainThread:
      case nsTaskPriority::ThisFrameMainThread:
      case nsTaskPriority::ENUM_COUNT:
        // nothing to do for these enum values
        break;
    }
  }
}

void nsTaskSystem::DependencyHasFinished(nsTaskGroup* pGroup)
{
  // remove one dependency from the group
  if (pGroup->m_iNumActiveDependencies.Decrement() == 0)
  {
    // if there are no remaining dependencies, kick off all tasks in this group
    ScheduleGroupTasks(pGroup, true);
  }
}

nsResult nsTaskSystem::CancelGroup(nsTaskGroupID group, nsOnTaskRunning::Enum onTaskRunning)
{
  if (nsTaskSystem::IsTaskGroupFinished(group))
    return NS_SUCCESS;

  NS_PROFILE_SCOPE("CancelGroup");

  NS_LOCK(s_TaskSystemMutex);

  nsResult res = NS_SUCCESS;

  auto TasksCopy = group.m_pTaskGroup->m_Tasks;

  // first cancel ALL the tasks in the group, without waiting for anything
  for (nsUInt32 task = 0; task < TasksCopy.GetCount(); ++task)
  {
    if (CancelTask(TasksCopy[task], nsOnTaskRunning::ReturnWithoutBlocking) == NS_FAILURE)
    {
      res = NS_FAILURE;
    }
  }

  // if all tasks could be removed without problems, we do not need to try it again with blocking

  if (onTaskRunning == nsOnTaskRunning::WaitTillFinished && res == NS_FAILURE)
  {
    // now cancel the tasks in the group again, this time wait for those that are already running
    for (nsUInt32 task = 0; task < TasksCopy.GetCount(); ++task)
    {
      CancelTask(TasksCopy[task], nsOnTaskRunning::WaitTillFinished).IgnoreResult();
    }
  }

  return res;
}

void nsTaskSystem::WaitForGroup(nsTaskGroupID group)
{
  NS_PROFILE_SCOPE("WaitForGroup");

  NS_ASSERT_DEV(tl_TaskWorkerInfo.m_bAllowNestedTasks, "The executing task '{}' is flagged to never wait for other tasks but does so anyway. Remove the flag or remove the wait-dependency.", tl_TaskWorkerInfo.m_szTaskName);

  const auto ThreadTaskType = tl_TaskWorkerInfo.m_WorkerType;
  const bool bAllowSleep = ThreadTaskType != nsWorkerThreadType::MainThread;

  while (!nsTaskSystem::IsTaskGroupFinished(group))
  {
    if (!HelpExecutingTasks(group))
    {
      if (bAllowSleep)
      {
        const nsWorkerThreadType::Enum typeToWakeUp = (ThreadTaskType == nsWorkerThreadType::Unknown) ? nsWorkerThreadType::ShortTasks : ThreadTaskType;

        if (tl_TaskWorkerInfo.m_pWorkerState)
        {
          NS_VERIFY(tl_TaskWorkerInfo.m_pWorkerState->Set((int)nsTaskWorkerState::Blocked) == (int)nsTaskWorkerState::Active, "Corrupt worker state");
        }

        WakeUpThreads(typeToWakeUp, 1);

        group.m_pTaskGroup->WaitForFinish(group);

        if (tl_TaskWorkerInfo.m_pWorkerState)
        {
          NS_VERIFY(tl_TaskWorkerInfo.m_pWorkerState->Set((int)nsTaskWorkerState::Active) == (int)nsTaskWorkerState::Blocked, "Corrupt worker state");
        }

        break;
      }
      else
      {
        nsThreadUtils::YieldTimeSlice();
      }
    }
  }
}

void nsTaskSystem::WaitForCondition(nsDelegate<bool()> condition)
{
  NS_PROFILE_SCOPE("WaitForCondition");

  NS_ASSERT_DEV(tl_TaskWorkerInfo.m_bAllowNestedTasks, "The executing task '{}' is flagged to never wait for other tasks but does so anyway. Remove the flag or remove the wait-dependency.", tl_TaskWorkerInfo.m_szTaskName);

  const auto ThreadTaskType = tl_TaskWorkerInfo.m_WorkerType;
  const bool bAllowSleep = ThreadTaskType != nsWorkerThreadType::MainThread;

  while (!condition())
  {
    if (!HelpExecutingTasks(nsTaskGroupID()))
    {
      if (bAllowSleep)
      {
        const nsWorkerThreadType::Enum typeToWakeUp = (ThreadTaskType == nsWorkerThreadType::Unknown) ? nsWorkerThreadType::ShortTasks : ThreadTaskType;

        if (tl_TaskWorkerInfo.m_pWorkerState)
        {
          NS_VERIFY(tl_TaskWorkerInfo.m_pWorkerState->Set((int)nsTaskWorkerState::Blocked) == (int)nsTaskWorkerState::Active, "Corrupt worker state");
        }

        WakeUpThreads(typeToWakeUp, 1);

        while (!condition())
        {
          // TODO: busy loop for now
          nsThreadUtils::YieldTimeSlice();
        }

        if (tl_TaskWorkerInfo.m_pWorkerState)
        {
          NS_VERIFY(tl_TaskWorkerInfo.m_pWorkerState->Set((int)nsTaskWorkerState::Active) == (int)nsTaskWorkerState::Blocked, "Corrupt worker state");
        }

        break;
      }
      else
      {
        nsThreadUtils::YieldTimeSlice();
      }
    }
  }
}
