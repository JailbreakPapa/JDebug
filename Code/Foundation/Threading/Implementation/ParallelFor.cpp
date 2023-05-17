#include <Foundation/FoundationPCH.h>

#include <Foundation/Threading/TaskSystem.h>

/// \brief This is a helper class that splits up task items via index ranges.
template <typename IndexType, typename Callback>
class IndexedTask final : public wdTask
{
public:
  IndexedTask(IndexType uiStartIndex, IndexType uiNumItems, Callback taskCallback, IndexType uiItemsPerInvocation)
    : m_uiStartIndex(uiStartIndex)
    , m_uiNumItems(uiNumItems)
    , m_uiItemsPerInvocation(uiItemsPerInvocation)
    , m_TaskCallback(std::move(taskCallback))
  {
  }

  void Execute() override
  {
    // Work through all of them.
    m_TaskCallback(m_uiStartIndex, m_uiStartIndex + m_uiNumItems);
  }

  void ExecuteWithMultiplicity(wdUInt32 uiInvocation) const override
  {
    const IndexType uiSliceStartIndex = uiInvocation * m_uiItemsPerInvocation;
    const IndexType uiSliceEndIndex = wdMath::Min(uiSliceStartIndex + m_uiItemsPerInvocation, m_uiStartIndex + m_uiNumItems);

    WD_ASSERT_DEV(uiSliceStartIndex < uiSliceEndIndex, "ParallelFor start/end indices given to index task are invalid: {} -> {}", uiSliceStartIndex, uiSliceEndIndex);

    // Run through the calculated slice, the end index is exclusive, i.e., should not be handled by this instance.
    m_TaskCallback(uiSliceStartIndex, uiSliceEndIndex);
  }

private:
  IndexType m_uiStartIndex;
  IndexType m_uiNumItems;
  IndexType m_uiItemsPerInvocation;
  Callback m_TaskCallback;
};

template <typename IndexType, typename Callback>
void ParallelForIndexedInternal(IndexType uiStartIndex, IndexType uiNumItems, const Callback& taskCallback, const char* szTaskName, const wdParallelForParams& params)
{
  typedef IndexedTask<IndexType, Callback> Task;

  if (!szTaskName)
  {
    szTaskName = "Generic Indexed Task";
  }

  if (uiNumItems <= params.m_uiBinSize)
  {
    // If we have not exceeded the threading threshold we use serial execution

    Task indexedTask(uiStartIndex, uiNumItems, std::move(taskCallback), uiNumItems);
    indexedTask.ConfigureTask(szTaskName, wdTaskNesting::Never);

    WD_PROFILE_SCOPE(szTaskName);
    indexedTask.Execute();
  }
  else
  {
    wdUInt32 uiMultiplicity;
    wdUInt64 uiItemsPerInvocation;
    params.DetermineThreading(uiNumItems, uiMultiplicity, uiItemsPerInvocation);

    wdAllocatorBase* pAllocator = (params.m_pTaskAllocator != nullptr) ? params.m_pTaskAllocator : wdFoundation::GetDefaultAllocator();

    wdSharedPtr<Task> pIndexedTask = WD_NEW(pAllocator, Task, uiStartIndex, uiNumItems, std::move(taskCallback), static_cast<IndexType>(uiItemsPerInvocation));
    pIndexedTask->ConfigureTask(szTaskName, wdTaskNesting::Never);

    pIndexedTask->SetMultiplicity(uiMultiplicity);
    wdTaskGroupID taskGroupId = wdTaskSystem::StartSingleTask(pIndexedTask, wdTaskPriority::EarlyThisFrame);
    wdTaskSystem::WaitForGroup(taskGroupId);
  }
}

void wdParallelForParams::DetermineThreading(wdUInt64 uiNumItemsToExecute, wdUInt32& out_uiNumTasksToRun, wdUInt64& out_uiNumItemsPerTask) const
{
  // we create a single task, but we set it's multiplicity to M (= out_uiNumTasksToRun)
  // so that it gets scheduled M times, which is effectively the same as creating M tasks

  const wdUInt32 uiNumWorkerThreads = wdTaskSystem::GetWorkerThreadCount(wdWorkerThreadType::ShortTasks);
  const wdUInt64 uiMaxTasksToUse = uiNumWorkerThreads * m_uiMaxTasksPerThread;
  const wdUInt64 uiMaxExecutionsRequired = wdMath::Max(1llu, uiNumItemsToExecute / m_uiBinSize);

  if (uiMaxExecutionsRequired >= uiMaxTasksToUse)
  {
    // if we have more items to execute, than the upper limit of tasks that we want to spawn, clamp the number of tasks
    // and give each task more items to do
    out_uiNumTasksToRun = uiMaxTasksToUse & 0xFFFFFFFF;
  }
  else
  {
    // if we want to execute fewer items than we have tasks available, just run exactly as many tasks as we have items
    out_uiNumTasksToRun = uiMaxExecutionsRequired & 0xFFFFFFFF;
  }

  // now that we determined the number of tasks to run, compute how much each task should do
  out_uiNumItemsPerTask = uiNumItemsToExecute / out_uiNumTasksToRun;

  // due to rounding down in the line above, it can happen that we would execute too few tasks
  if (out_uiNumItemsPerTask * out_uiNumTasksToRun < uiNumItemsToExecute)
  {
    // to fix this, either do one more task invocation, or one more item per task
    if (out_uiNumItemsPerTask * (out_uiNumTasksToRun + 1) >= uiNumItemsToExecute)
    {
      ++out_uiNumTasksToRun;

      // though with one more task we may execute too many items, so if possible reduce the number of items that each task executes
      while ((out_uiNumItemsPerTask - 1) * out_uiNumTasksToRun >= uiNumItemsToExecute)
      {
        --out_uiNumItemsPerTask;
      }
    }
    else
    {
      ++out_uiNumItemsPerTask;

      // though if every task executes one more item, we may execute too many items, so if possible reduce the number of tasks again
      while (out_uiNumItemsPerTask * (out_uiNumTasksToRun - 1) >= uiNumItemsToExecute)
      {
        --out_uiNumTasksToRun;
      }
    }

    WD_ASSERT_DEV(out_uiNumItemsPerTask * out_uiNumTasksToRun >= uiNumItemsToExecute, "wdParallelFor is missing invocations");
  }
}

void wdTaskSystem::ParallelForIndexed(wdUInt32 uiStartIndex, wdUInt32 uiNumItems, wdParallelForIndexedFunction32 taskCallback, const char* szTaskName, const wdParallelForParams& params)
{
  ParallelForIndexedInternal<wdUInt32, wdParallelForIndexedFunction32>(uiStartIndex, uiNumItems, taskCallback, szTaskName, params);
}

void wdTaskSystem::ParallelForIndexed(wdUInt64 uiStartIndex, wdUInt64 uiNumItems, wdParallelForIndexedFunction64 taskCallback, const char* szTaskName, const wdParallelForParams& params)
{
  ParallelForIndexedInternal<wdUInt64, wdParallelForIndexedFunction64>(uiStartIndex, uiNumItems, taskCallback, szTaskName, params);
}

WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_ParallelFor);
