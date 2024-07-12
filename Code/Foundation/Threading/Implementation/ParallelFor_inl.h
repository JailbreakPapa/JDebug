#pragma once

#include <Foundation/Memory/FrameAllocator.h>
#include <Foundation/Profiling/Profiling.h>

template <typename ElemType>
class ArrayPtrTask final : public nsTask
{
public:
  ArrayPtrTask(nsArrayPtr<ElemType> payload, nsParallelForFunction<ElemType> taskCallback, nsUInt32 uiItemsPerInvocation)
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

  void ExecuteWithMultiplicity(nsUInt32 uiInvocation) const override
  {
    const nsUInt32 uiSliceStartIndex = uiInvocation * m_uiItemsPerInvocation;

    const nsUInt32 uiRemainingItems = uiSliceStartIndex > m_Payload.GetCount() ? 0 : m_Payload.GetCount() - uiSliceStartIndex;
    const nsUInt32 uiSliceItemCount = nsMath::Min(m_uiItemsPerInvocation, uiRemainingItems);

    if (uiSliceItemCount > 0)
    {
      // Run through the calculated slice.
      auto taskItemSlice = m_Payload.GetSubArray(uiSliceStartIndex, uiSliceItemCount);
      m_TaskCallback(uiSliceStartIndex, taskItemSlice);
    }
  }

private:
  nsArrayPtr<ElemType> m_Payload;
  nsUInt32 m_uiItemsPerInvocation;
  nsParallelForFunction<ElemType> m_TaskCallback;
};

template <typename ElemType>
void nsTaskSystem::ParallelForInternal(nsArrayPtr<ElemType> taskItems, nsParallelForFunction<ElemType> taskCallback, const char* taskName, const nsParallelForParams& params)
{
  if (taskItems.GetCount() <= params.m_uiBinSize)
  {
    ArrayPtrTask<ElemType> arrayPtrTask(taskItems, std::move(taskCallback), taskItems.GetCount());
    arrayPtrTask.ConfigureTask(taskName ? taskName : "Generic ArrayPtr Task", params.m_NestingMode);

    NS_PROFILE_SCOPE(arrayPtrTask.m_sTaskName);
    arrayPtrTask.Execute();
  }
  else
  {
    nsUInt32 uiMultiplicity;
    nsUInt64 uiItemsPerInvocation;
    params.DetermineThreading(taskItems.GetCount(), uiMultiplicity, uiItemsPerInvocation);

    nsAllocator* pAllocator = (params.m_pTaskAllocator != nullptr) ? params.m_pTaskAllocator : nsFoundation::GetDefaultAllocator();

    nsSharedPtr<ArrayPtrTask<ElemType>> pArrayPtrTask = NS_NEW(pAllocator, ArrayPtrTask<ElemType>, taskItems, std::move(taskCallback), static_cast<nsUInt32>(uiItemsPerInvocation));
    pArrayPtrTask->ConfigureTask(taskName ? taskName : "Generic ArrayPtr Task", params.m_NestingMode);

    pArrayPtrTask->SetMultiplicity(uiMultiplicity);
    nsTaskGroupID taskGroupId = nsTaskSystem::StartSingleTask(pArrayPtrTask, nsTaskPriority::EarlyThisFrame);
    nsTaskSystem::WaitForGroup(taskGroupId);
  }
}

template <typename ElemType, typename Callback>
void nsTaskSystem::ParallelFor(nsArrayPtr<ElemType> taskItems, Callback taskCallback, const char* szTaskName, const nsParallelForParams& params)
{
  auto wrappedCallback = [taskCallback = std::move(taskCallback)](
                           nsUInt32 /*uiBaseIndex*/, nsArrayPtr<ElemType> taskSlice)
  { taskCallback(taskSlice); };

  ParallelForInternal<ElemType>(
    taskItems, nsParallelForFunction<ElemType>(std::move(wrappedCallback), nsFrameAllocator::GetCurrentAllocator()), szTaskName, params);
}

template <typename ElemType, typename Callback>
void nsTaskSystem::ParallelForSingle(nsArrayPtr<ElemType> taskItems, Callback taskCallback, const char* szTaskName, const nsParallelForParams& params)
{
  auto wrappedCallback = [taskCallback = std::move(taskCallback)](nsUInt32 /*uiBaseIndex*/, nsArrayPtr<ElemType> taskSlice)
  {
    // Handing in by non-const& allows to use callbacks with (non-)const& as well as value parameters.
    for (ElemType& taskItem : taskSlice)
    {
      taskCallback(taskItem);
    }
  };

  ParallelForInternal<ElemType>(
    taskItems, nsParallelForFunction<ElemType>(std::move(wrappedCallback), nsFrameAllocator::GetCurrentAllocator()), szTaskName, params);
}

template <typename ElemType, typename Callback>
void nsTaskSystem::ParallelForSingleIndex(
  nsArrayPtr<ElemType> taskItems, Callback taskCallback, const char* szTaskName, const nsParallelForParams& params)
{
  auto wrappedCallback = [taskCallback = std::move(taskCallback)](nsUInt32 uiBaseIndex, nsArrayPtr<ElemType> taskSlice)
  {
    for (nsUInt32 uiIndex = 0; uiIndex < taskSlice.GetCount(); ++uiIndex)
    {
      // Handing in by dereferenced pointer allows to use callbacks with (non-)const& as well as value parameters.
      taskCallback(uiBaseIndex + uiIndex, *(taskSlice.GetPtr() + uiIndex));
    }
  };

  ParallelForInternal<ElemType>(
    taskItems, nsParallelForFunction<ElemType>(std::move(wrappedCallback), nsFrameAllocator::GetCurrentAllocator()), szTaskName, params);
}
