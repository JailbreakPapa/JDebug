#pragma once

#include <Foundation/Threading/TaskSystem.h>

class nsTaskSystemThreadState
{
private:
  friend class nsTaskSystem;
  friend class nsTaskWorkerThread;

  // The arrays of all the active worker threads.
  nsDynamicArray<nsTaskWorkerThread*> m_Workers[nsWorkerThreadType::ENUM_COUNT];

  // the number of allocated (non-null) worker threads in m_Workers
  nsAtomicInteger32 m_iAllocatedWorkers[nsWorkerThreadType::ENUM_COUNT];

  // the maximum number of worker threads that should be non-idle (and not blocked) at any time
  nsUInt32 m_uiMaxWorkersToUse[nsWorkerThreadType::ENUM_COUNT] = {};
};

class nsTaskSystemState
{
private:
  friend class nsTaskSystem;

  // The target frame time used by FinishFrameTasks()
  nsTime m_TargetFrameTime = nsTime::MakeFromSeconds(1.0 / 40.0); // => 25 ms

  // The deque can grow without relocating existing data, therefore the nsTaskGroupID's can store pointers directly to the data
  nsDeque<nsTaskGroup> m_TaskGroups;

  // The lists of all scheduled tasks, for each priority.
  nsList<nsTaskSystem::TaskData> m_Tasks[nsTaskPriority::ENUM_COUNT];
};
