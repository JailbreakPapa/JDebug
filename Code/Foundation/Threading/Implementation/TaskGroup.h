#pragma once

#include <Foundation/Strings/String.h>
#include <Foundation/Threading/ConditionVariable.h>
#include <Foundation/Threading/Implementation/TaskSystemDeclarations.h>
#include <Foundation/Types/SharedPtr.h>

/// \internal Represents the state of a group of tasks that can be waited on
class nsTaskGroup
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsTaskGroup);

public:
  nsTaskGroup();
  ~nsTaskGroup();

private:
  friend class nsTaskSystem;

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  static void DebugCheckTaskGroup(nsTaskGroupID groupID, nsMutex& mutex);
#else
  NS_ALWAYS_INLINE static void DebugCheckTaskGroup(nsTaskGroupID groupID, nsMutex& mutex)
  {
  }
#endif

  /// \brief Puts the calling thread to sleep until this group is fully finished.
  void WaitForFinish(nsTaskGroupID group) const;
  void Reuse(nsTaskPriority::Enum priority, nsOnTaskGroupFinishedCallback callback);

  bool m_bInUse = true;
  bool m_bStartedByUser = false;
  nsUInt16 m_uiTaskGroupIndex = 0xFFFF; // only there as a debugging aid
  nsUInt32 m_uiGroupCounter = 1;
  nsHybridArray<nsSharedPtr<nsTask>, 16> m_Tasks;
  nsHybridArray<nsTaskGroupID, 4> m_DependsOnGroups;
  nsHybridArray<nsTaskGroupID, 8> m_OthersDependingOnMe;
  nsAtomicInteger32 m_iNumActiveDependencies;
  nsAtomicInteger32 m_iNumRemainingTasks;
  nsOnTaskGroupFinishedCallback m_OnFinishedCallback;
  nsTaskPriority::Enum m_Priority = nsTaskPriority::ThisFrame;
  mutable nsConditionVariable m_CondVarGroupFinished;
};
