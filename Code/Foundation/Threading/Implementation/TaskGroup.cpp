#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/Implementation/Task.h>
#include <Foundation/Threading/Implementation/TaskGroup.h>
#include <Foundation/Threading/Lock.h>

wdTaskGroup::wdTaskGroup() = default;
wdTaskGroup::~wdTaskGroup() = default;

void wdTaskGroup::WaitForFinish(wdTaskGroupID group) const
{
  if (m_uiGroupCounter != group.m_uiGroupCounter)
    return;

  WD_LOCK(m_CondVarGroupFinished);

  while (m_uiGroupCounter == group.m_uiGroupCounter)
  {
    m_CondVarGroupFinished.UnlockWaitForSignalAndLock();
  }
}

void wdTaskGroup::Reuse(wdTaskPriority::Enum priority, wdOnTaskGroupFinishedCallback callback)
{
  m_bInUse = true;
  m_bStartedByUser = false;
  m_uiGroupCounter += 2; // even if it wraps around, it will never be zero, thus zero stays an invalid group counter
  m_Tasks.Clear();
  m_DependsOnGroups.Clear();
  m_OthersDependingOnMe.Clear();
  m_Priority = priority;
  m_OnFinishedCallback = callback;
}

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
void wdTaskGroup::DebugCheckTaskGroup(wdTaskGroupID groupID, wdMutex& mutex)
{
  WD_LOCK(mutex);

  const wdTaskGroup* pGroup = groupID.m_pTaskGroup;

  WD_ASSERT_DEV(pGroup != nullptr, "TaskGroupID is invalid.");
  WD_ASSERT_DEV(pGroup->m_uiGroupCounter == groupID.m_uiGroupCounter, "The given TaskGroupID is not valid anymore.");
  WD_ASSERT_DEV(!pGroup->m_bStartedByUser, "The given TaskGroupID is already started, you cannot modify it anymore.");
  WD_ASSERT_DEV(pGroup->m_iNumActiveDependencies == 0, "Invalid active dependenices");
}
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_TaskGroup);
