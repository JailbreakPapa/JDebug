#pragma once

#include <Foundation/Strings/String.h>
#include <Foundation/Threading/ConditionVariable.h>
#include <Foundation/Threading/Implementation/TaskSystemDeclarations.h>
#include <Foundation/Types/SharedPtr.h>

/// \internal Represents the state of a group of tasks that can be waited on
class wdTaskGroup
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdTaskGroup);

public:
  wdTaskGroup();
  ~wdTaskGroup();

private:
  friend class wdTaskSystem;

#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  static void DebugCheckTaskGroup(wdTaskGroupID groupID, wdMutex& mutex);
#else
  WD_ALWAYS_INLINE static void DebugCheckTaskGroup(wdTaskGroupID groupID, wdMutex& mutex) {}
#endif

  /// \brief Puts the calling thread to sleep until this group is fully finished.
  void WaitForFinish(wdTaskGroupID group) const;
  void Reuse(wdTaskPriority::Enum priority, wdOnTaskGroupFinishedCallback callback);

  bool m_bInUse = true;
  bool m_bStartedByUser = false;
  wdUInt16 m_uiTaskGroupIndex = 0xFFFF; // only there as a debugging aid
  wdUInt32 m_uiGroupCounter = 1;
  wdHybridArray<wdSharedPtr<wdTask>, 16> m_Tasks;
  wdHybridArray<wdTaskGroupID, 4> m_DependsOnGroups;
  wdHybridArray<wdTaskGroupID, 8> m_OthersDependingOnMe;
  wdAtomicInteger32 m_iNumActiveDependencies;
  wdAtomicInteger32 m_iNumRemainingTasks;
  wdOnTaskGroupFinishedCallback m_OnFinishedCallback;
  wdTaskPriority::Enum m_Priority = wdTaskPriority::ThisFrame;
  mutable wdConditionVariable m_CondVarGroupFinished;
};
