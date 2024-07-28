#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>

nsExtractedRenderData::nsExtractedRenderData() = default;

void nsExtractedRenderData::AddRenderData(const nsRenderData* pRenderData, nsRenderData::Category category)
{
  m_DataPerCategory.EnsureCount(category.m_uiValue + 1);

  auto& sortableRenderData = m_DataPerCategory[category.m_uiValue].m_SortableRenderData.ExpandAndGetRef();
  sortableRenderData.m_pRenderData = pRenderData;
  sortableRenderData.m_uiSortingKey = pRenderData->GetCategorySortingKey(category, m_Camera);
}

void nsExtractedRenderData::AddFrameData(const nsRenderData* pFrameData)
{
  m_FrameData.PushBack(pFrameData);
}

void nsExtractedRenderData::SortAndBatch()
{
  NS_PROFILE_SCOPE("SortAndBatch");

  struct RenderDataComparer
  {
    NS_FORCE_INLINE bool Less(const nsRenderDataBatch::SortableRenderData& a, const nsRenderDataBatch::SortableRenderData& b) const
    {
      if (a.m_uiSortingKey == b.m_uiSortingKey)
      {
        return a.m_pRenderData->m_uiBatchId < b.m_pRenderData->m_uiBatchId;
      }

      return a.m_uiSortingKey < b.m_uiSortingKey;
    }
  };

  for (auto& dataPerCategory : m_DataPerCategory)
  {
    if (dataPerCategory.m_SortableRenderData.IsEmpty())
      continue;

    auto& data = dataPerCategory.m_SortableRenderData;

    // Sort
    data.Sort(RenderDataComparer());

    // Find batches
    nsUInt32 uiCurrentBatchId = data[0].m_pRenderData->m_uiBatchId;
    nsUInt32 uiCurrentBatchStartIndex = 0;
    const nsRTTI* pCurrentBatchType = data[0].m_pRenderData->GetDynamicRTTI();

    for (nsUInt32 i = 1; i < data.GetCount(); ++i)
    {
      auto pRenderData = data[i].m_pRenderData;

      if (pRenderData->m_uiBatchId != uiCurrentBatchId || pRenderData->GetDynamicRTTI() != pCurrentBatchType)
      {
        dataPerCategory.m_Batches.ExpandAndGetRef().m_Data = nsMakeArrayPtr(&data[uiCurrentBatchStartIndex], i - uiCurrentBatchStartIndex);

        uiCurrentBatchId = pRenderData->m_uiBatchId;
        uiCurrentBatchStartIndex = i;
        pCurrentBatchType = pRenderData->GetDynamicRTTI();
      }
    }

    dataPerCategory.m_Batches.ExpandAndGetRef().m_Data = nsMakeArrayPtr(&data[uiCurrentBatchStartIndex], data.GetCount() - uiCurrentBatchStartIndex);
  }
}

void nsExtractedRenderData::Clear()
{
  for (auto& dataPerCategory : m_DataPerCategory)
  {
    dataPerCategory.m_Batches.Clear();
    dataPerCategory.m_SortableRenderData.Clear();
  }

  m_FrameData.Clear();

  // TODO: intelligent compact
}

nsRenderDataBatchList nsExtractedRenderData::GetRenderDataBatchesWithCategory(nsRenderData::Category category, nsRenderDataBatch::Filter filter) const
{
  if (category.m_uiValue < m_DataPerCategory.GetCount())
  {
    nsRenderDataBatchList list;
    list.m_Batches = m_DataPerCategory[category.m_uiValue].m_Batches;
    list.m_Filter = filter;

    return list;
  }

  return nsRenderDataBatchList();
}

const nsRenderData* nsExtractedRenderData::GetFrameData(const nsRTTI* pRtti) const
{
  for (auto pData : m_FrameData)
  {
    if (pData->IsInstanceOf(pRtti))
    {
      return pData;
    }
  }

  return nullptr;
}
