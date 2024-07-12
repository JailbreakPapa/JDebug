#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/Implementation/Task.h>
#include <Foundation/Threading/Implementation/TaskGroup.h>
#include <Foundation/Threading/Lock.h>

nsTaskGroup::nsTaskGroup() = default;
nsTaskGroup::~nsTaskGroup() = default;

void nsTaskGroup::WaitForFinish(nsTaskGroupID group) const
{
  if (m_uiGroupCounter != group.m_uiGroupCounter)
    return;

  NS_LOCK(m_CondVarGroupFinished);

  while (m_uiGroupCounter == group.m_uiGroupCounter)
  {
    m_CondVarGroupFinished.UnlockWaitForSignalAndLock();
  }
}

void nsTaskGroup::Reuse(nsTaskPriority::Enum priority, nsOnTaskGroupFinishedCallback callback)
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

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
void nsTaskGroup::DebugCheckTaskGroup(nsTaskGroupID groupID, nsMutex& mutex)
{
  NS_LOCK(mutex);

  const nsTaskGroup* pGroup = groupID.m_pTaskGroup;

  NS_ASSERT_DEV(pGroup != nullptr, "TaskGroupID is invalid.");
  NS_ASSERT_DEV(pGroup->m_uiGroupCounter == groupID.m_uiGroupCounter, "The given TaskGroupID is not valid anymore.");
  NS_ASSERT_DEV(!pGroup->m_bStartedByUser, "The given TaskGroupID is already started, you cannot modify it anymore.");
  NS_ASSERT_DEV(pGroup->m_iNumActiveDependencies == 0, "Invalid active dependenices");
}
#endif
