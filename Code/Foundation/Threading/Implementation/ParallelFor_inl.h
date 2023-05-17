#pragma once

#include <Foundation/Memory/FrameAllocator.h>
#include <Foundation/Profiling/Profiling.h>

template <typename ElemType>
class ArrayPtrTask final : public wdTask
{
public:
  ArrayPtrTask(wdArrayPtr<ElemType> payload, wdParallelForFunction<ElemType> taskCallback, wdUInt32 uiItemsPerInvocation)
    : m_Payload(payload)
    , m_uiItemsPerInvocation(uiItemsPerInvocation)
    , m_TaskCallback(std::move(taskCallback))
  {
  }

  void Execute() override
  {
    // Work through all of them.
    m_TaskCallback(0, m_Payload);
  }

  void ExecuteWithMultiplicity(wdUInt32 uiInvocation) const override
  {
    const wdUInt32 uiSliceStartIndex = uiInvocation * m_uiItemsPerInvocation;

    const wdUInt32 uiRemainingItems = uiSliceStartIndex > m_Payload.GetCount() ? 0 : m_Payload.GetCount() - uiSliceStartIndex;
    const wdUInt32 uiSliceItemCount = wdMath::Min(m_uiItemsPerInvocation, uiRemainingItems);

    if (uiSliceItemCount > 0)
    {
      // Run through the calculated slice.
      auto taskItemSlice = m_Payload.GetSubArray(uiSliceStartIndex, uiSliceItemCount);
      m_TaskCallback(uiSliceStartIndex, taskItemSlice);
    }
  }

private:
  wdArrayPtr<ElemType> m_Payload;
  wdUInt32 m_uiItemsPerInvocation;
  wdParallelForFunction<ElemType> m_TaskCallback;
};

template <typename ElemType>
void wdTaskSystem::ParallelForInternal(wdArrayPtr<ElemType> taskItems, wdParallelForFunction<ElemType> taskCallback, const char* taskName, const wdParallelForParams& params)
{
  if (taskItems.GetCount() <= params.m_uiBinSize)
  {
    ArrayPtrTask<ElemType> arrayPtrTask(taskItems, std::move(taskCallback), taskItems.GetCount());
    arrayPtrTask.ConfigureTask(taskName ? taskName : "Generic ArrayPtr Task", params.m_NestingMode);

    WD_PROFILE_SCOPE(arrayPtrTask.m_sTaskName);
    arrayPtrTask.Execute();
  }
  else
  {
    wdUInt32 uiMultiplicity;
    wdUInt64 uiItemsPerInvocation;
    params.DetermineThreading(taskItems.GetCount(), uiMultiplicity, uiItemsPerInvocation);

    wdAllocatorBase* pAllocator = (params.m_pTaskAllocator != nullptr) ? params.m_pTaskAllocator : wdFoundation::GetDefaultAllocator();

    wdSharedPtr<ArrayPtrTask<ElemType>> pArrayPtrTask = WD_NEW(pAllocator, ArrayPtrTask<ElemType>, taskItems, std::move(taskCallback), static_cast<wdUInt32>(uiItemsPerInvocation));
    pArrayPtrTask->ConfigureTask(taskName ? taskName : "Generic ArrayPtr Task", params.m_NestingMode);

    pArrayPtrTask->SetMultiplicity(uiMultiplicity);
    wdTaskGroupID taskGroupId = wdTaskSystem::StartSingleTask(pArrayPtrTask, wdTaskPriority::EarlyThisFrame);
    wdTaskSystem::WaitForGroup(taskGroupId);
  }
}

template <typename ElemType, typename Callback>
void wdTaskSystem::ParallelFor(wdArrayPtr<ElemType> taskItems, Callback taskCallback, const char* szTaskName, const wdParallelForParams& params)
{
  auto wrappedCallback = [taskCallback = std::move(taskCallback)](
                           wdUInt32 /*uiBaseIndex*/, wdArrayPtr<ElemType> taskSlice) { taskCallback(taskSlice); };

  ParallelForInternal<ElemType>(
    taskItems, wdParallelForFunction<ElemType>(std::move(wrappedCallback), wdFrameAllocator::GetCurrentAllocator()), szTaskName, params);
}

template <typename ElemType, typename Callback>
void wdTaskSystem::ParallelForSingle(wdArrayPtr<ElemType> taskItems, Callback taskCallback, const char* szTaskName, const wdParallelForParams& params)
{
  auto wrappedCallback = [taskCallback = std::move(taskCallback)](wdUInt32 /*uiBaseIndex*/, wdArrayPtr<ElemType> taskSlice) {
    // Handing in by non-const& allows to use callbacks with (non-)const& as well as value parameters.
    for (ElemType& taskItem : taskSlice)
    {
      taskCallback(taskItem);
    }
  };

  ParallelForInternal<ElemType>(
    taskItems, wdParallelForFunction<ElemType>(std::move(wrappedCallback), wdFrameAllocator::GetCurrentAllocator()), szTaskName, params);
}

template <typename ElemType, typename Callback>
void wdTaskSystem::ParallelForSingleIndex(
  wdArrayPtr<ElemType> taskItems, Callback taskCallback, const char* szTaskName, const wdParallelForParams& params)
{
  auto wrappedCallback = [taskCallback = std::move(taskCallback)](wdUInt32 uiBaseIndex, wdArrayPtr<ElemType> taskSlice) {
    for (wdUInt32 uiIndex = 0; uiIndex < taskSlice.GetCount(); ++uiIndex)
    {
      // Handing in by dereferenced pointer allows to use callbacks with (non-)const& as well as value parameters.
      taskCallback(uiBaseIndex + uiIndex, *(taskSlice.GetPtr() + uiIndex));
    }
  };

  ParallelForInternal<ElemType>(
    taskItems, wdParallelForFunction<ElemType>(std::move(wrappedCallback), wdFrameAllocator::GetCurrentAllocator()), szTaskName, params);
}
