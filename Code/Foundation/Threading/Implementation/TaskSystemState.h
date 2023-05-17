#pragma once

#include <Foundation/Threading/TaskSystem.h>

class wdTaskSystemThreadState
{
private:
  friend class wdTaskSystem;
  friend class wdTaskWorkerThread;

  // The arrays of all the active worker threads.
  wdDynamicArray<wdTaskWorkerThread*> m_Workers[wdWorkerThreadType::ENUM_COUNT];

  // the number of allocated (non-null) worker threads in m_Workers
  wdAtomicInteger32 m_iAllocatedWorkers[wdWorkerThreadType::ENUM_COUNT];

  // the maximum number of worker threads that should be non-idle (and not blocked) at any time
  wdUInt32 m_uiMaxWorkersToUse[wdWorkerThreadType::ENUM_COUNT] = {};
};

class wdTaskSystemState
{
private:
  friend class wdTaskSystem;

  // The target frame time used by FinishFrameTasks()
  wdTime m_TargetFrameTime = wdTime::Seconds(1.0 / 40.0); // => 25 ms

  // The deque can grow without relocating existing data, therefore the wdTaskGroupID's can store pointers directly to the data
  wdDeque<wdTaskGroup> m_TaskGroups;

  // The lists of all scheduled tasks, for each priority.
  wdList<wdTaskSystem::TaskData> m_Tasks[wdTaskPriority::ENUM_COUNT];
};
