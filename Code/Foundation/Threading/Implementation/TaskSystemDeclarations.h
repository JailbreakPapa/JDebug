#pragma once

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Threading/ConditionVariable.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Delegate.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/UniquePtr.h>

class nsTask;
class nsTaskGroup;
class nsTaskWorkerThread;
class nsTaskSystemState;
class nsTaskSystemThreadState;
class nsDGMLGraph;
class nsAllocator;

/// \brief Describes the priority with which to execute a task.
///
/// For tasks that you start this frame and that need to finish within the same frame,
/// use 'EarlyThisFrame', 'ThisFrame' or 'LateThisFrame'.\n
/// However you should generally not rely on starting tasks in the same frame in which they need to finish.\n
/// Instead prefer to start a task, whose result you need in the next frame or even later.\n
/// For those tasks, use 'EarlyNextFrame', 'NextFrame', 'LateNextFrame' and 'InNFrames'.\n
/// Once 'nsTaskSystem::FinishFrameTasks' is called, all those tasks will be moved into the 'XYZThisFrame' categories.\n
/// For tasks that run over a longer period (e.g. path searches, procedural data creation), use 'LongRunning'.
/// Only use 'LongRunningHighPriority' for tasks that occur rarely, otherwise 'LongRunning' tasks might not get processed, at all.\n
/// For tasks that need to access files, prefer to use 'FileAccess', this way all file accesses get executed sequentially.\n
/// Use 'FileAccessHighPriority' to get very important file accesses done sooner. For example writing out a save-game should finish
/// quickly.\n For tasks that need to execute on the main thread (e.g. uploading GPU resources) use 'ThisFrameMainThread' or
/// 'SomeFrameMainThread' depending on how urgent it is. 'SomeFrameMainThread' tasks might get delayed for quite a while, depending on the
/// system load. For tasks that need to do several things (e.g. reading from a file AND uploading something on the main thread), split them
/// up into several tasks that depend on each other (or just let the first task start the next step once it is finished). There is no
/// guarantee WHEN (within a frame) main thread tasks get executed. Most of them will get executed upon a 'FinishFrameTasks' call. However
/// they are also run whenever the main thread needs to wait or cancel some other task and has nothing else to do. So tasks that get
/// executed on the main thread should never assume a certain state of other systems.
struct nsTaskPriority
{
  // clang-format off
  enum Enum : nsUInt8
  {
    EarlyThisFrame,           ///< Highest priority, guaranteed to get finished in this frame.
    ThisFrame,                ///< Medium priority, guaranteed to get finished in this frame.
    LateThisFrame,            ///< Low priority, guaranteed to get finished in this frame.
    EarlyNextFrame,           ///< Highest priority in next frame, guaranteed to get finished this frame or the next.
    NextFrame,                ///< Medium priority in next frame, guaranteed to get finished this frame or the next.
    LateNextFrame,            ///< Low priority in next frame, guaranteed to get finished this frame or the next.

    In2Frames,                ///< A short task that should be finished no later than in 2 frames
    In3Frames,                ///< A short task that should be finished no later than in 3 frames
    In4Frames,                ///< A short task that should be finished no later than in 4 frames
    In5Frames,                ///< A short task that should be finished no later than in 5 frames
    In6Frames,                ///< A short task that should be finished no later than in 6 frames
    In7Frames,                ///< A short task that should be finished no later than in 7 frames
    In8Frames,                ///< A short task that should be finished no later than in 8 frames
    In9Frames,                ///< A short task that should be finished no later than in 9 frames

    LongRunningHighPriority,  ///< Tasks that might take a while, but should be preferred over 'LongRunning' tasks. Use this priority only
                              ///< rarely, otherwise 'LongRunning' tasks might never get executed.
    LongRunning,              ///< Use this priority for tasks that might run for a while.
    FileAccessHighPriority,   ///< For tasks that require file access (e.g. resource loading). They run on one dedicated thread, such that
                              ///< file accesses are done sequentially and never in parallel.
    FileAccess,               ///< For tasks that require file access (e.g. resource loading). They run on one dedicated thread, such that file accesses
                              ///< are done sequentially and never in parallel.
    ThisFrameMainThread,      ///< Tasks that need to be executed this frame, but in the main thread. This is mostly intended for resource
                              ///< creation.
    SomeFrameMainThread,      ///< Tasks that have no hard deadline but need to be executed in the main thread. This is mostly intended for
                              ///< resource creation.

    ENUM_COUNT
  };
  // clang-format on
};

/// \brief Enum that describes what to do when waiting for or canceling tasks, that have already started execution.
struct nsOnTaskRunning
{
  enum Enum : nsUInt8
  {
    WaitTillFinished,
    ReturnWithoutBlocking
  };
};

/// \internal Enum that lists the different task worker thread types.
struct nsWorkerThreadType
{
  enum Enum : nsUInt8
  {
    Unknown,    ///< Default for all non-nsTaskSystem-worker threads. Will only execute short tasks.
    MainThread, ///< May only be used by the main thread (automatically used by the nsTaskSystem)
    ShortTasks,
    LongTasks,
    FileAccess,
    ENUM_COUNT
  };

  static const char* GetThreadTypeName(nsWorkerThreadType::Enum threadType);
};

/// \brief Given out by nsTaskSystem::CreateTaskGroup to identify a task group.
class NS_FOUNDATION_DLL nsTaskGroupID
{
public:
  NS_ALWAYS_INLINE nsTaskGroupID() = default;
  NS_ALWAYS_INLINE ~nsTaskGroupID() = default;

  /// \brief Returns false, if the GroupID does not reference a valid nsTaskGroup
  NS_ALWAYS_INLINE bool IsValid() const { return m_pTaskGroup != nullptr; }

  /// \brief Resets the GroupID into an invalid state.
  NS_ALWAYS_INLINE void Invalidate() { m_pTaskGroup = nullptr; }

  NS_ALWAYS_INLINE bool operator==(const nsTaskGroupID& other) const
  {
    return m_pTaskGroup == other.m_pTaskGroup && m_uiGroupCounter == other.m_uiGroupCounter;
  }
  NS_ALWAYS_INLINE bool operator!=(const nsTaskGroupID& other) const
  {
    return m_pTaskGroup != other.m_pTaskGroup || m_uiGroupCounter != other.m_uiGroupCounter;
  }
  NS_ALWAYS_INLINE bool operator<(const nsTaskGroupID& other) const
  {
    return m_pTaskGroup < other.m_pTaskGroup || (m_pTaskGroup == other.m_pTaskGroup && m_uiGroupCounter < other.m_uiGroupCounter);
  }

private:
  friend class nsTaskSystem;
  friend class nsTaskGroup;

  // the counter is used to determine whether this group id references the 'same' group, as m_pTaskGroup.
  // if m_pTaskGroup->m_uiGroupCounter is different to this->m_uiGroupCounter, then the group ID is not valid anymore.
  nsUInt32 m_uiGroupCounter = 0;

  // points to the actual task group object
  nsTaskGroup* m_pTaskGroup = nullptr;
};

/// \brief Callback type when a task group has been finished (or canceled).
using nsOnTaskGroupFinishedCallback = nsDelegate<void(nsTaskGroupID)>;

/// \brief Callback type when a task has been finished (or canceled).
using nsOnTaskFinishedCallback = nsDelegate<void(const nsSharedPtr<nsTask>&)>;

struct nsTaskGroupDependency
{
  NS_DECLARE_POD_TYPE();

  nsTaskGroupID m_TaskGroup;
  nsTaskGroupID m_DependsOn;
};

/// \brief Whether a task may wait for the completion of another task.
///
/// This is an optimization hint for the nsTaskSystem. Tasks that never wait on other tasks
/// can be executed more efficiently (without launching a dedicated thread), as they cannot produce
/// circular dependencies.
/// If the nesting specification is violated, the task system will assert.
enum class nsTaskNesting
{
  Maybe,
  Never,
};

/// \brief Settings for nsTaskSystem::ParallelFor invocations.
struct NS_FOUNDATION_DLL nsParallelForParams
{
  nsParallelForParams() = default; // do not remove, needed for Clang

  /// The minimum number of items that must be processed by a task instance.
  /// If the overall number of tasks lies below this value, all work will be executed purely serially
  /// without involving any tasks at all.
  nsUInt32 m_uiBinSize = 1;

  /// Indicates how many tasks per thread may be spawned at most by a ParallelFor invocation.
  /// Higher numbers give the scheduler more leeway to balance work across available threads.
  /// Generally, if all task items are expected to take basically the same amount of time,
  /// low numbers (usually 1) are recommended, while higher numbers (initially test with 2 or 3)
  /// might yield better results for workloads where task items may take vastly different amounts
  /// of time, such that scheduling in a balanced fashion becomes more difficult.
  nsUInt32 m_uiMaxTasksPerThread = 2;

  nsTaskNesting m_NestingMode = nsTaskNesting::Never;

  /// The allocator used to for the tasks that the parallel-for uses internally. If null, will use the default allocator.
  nsAllocator* m_pTaskAllocator = nullptr;

  void DetermineThreading(nsUInt64 uiNumItemsToExecute, nsUInt32& out_uiNumTasksToRun, nsUInt64& out_uiNumItemsPerTask) const;
};

using nsParallelForIndexedFunction32 = nsDelegate<void(nsUInt32, nsUInt32), 48>;
using nsParallelForIndexedFunction64 = nsDelegate<void(nsUInt64, nsUInt64), 48>;

template <typename ElemType>
using nsParallelForFunction = nsDelegate<void(nsUInt32, nsArrayPtr<ElemType>), 48>;

enum class nsTaskWorkerState
{
  Active = 0,
  Idle = 1,
  Blocked = 2,
};
