#pragma once

#include <Core/Graphics/Camera.h>
#include <RendererCore/Debug/DebugRendererContext.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/Pipeline/ViewData.h>

class NS_RENDERERCORE_DLL nsExtractedRenderData
{
public:
  nsExtractedRenderData();

  NS_ALWAYS_INLINE void SetCamera(const nsCamera& camera) { m_Camera = camera; }
  NS_ALWAYS_INLINE const nsCamera& GetCamera() const { return m_Camera; }

  NS_ALWAYS_INLINE void SetLodCamera(const nsCamera& camera) { m_LodCamera = camera; }
  NS_ALWAYS_INLINE const nsCamera& GetLodCamera() const { return m_LodCamera; }

  NS_ALWAYS_INLINE void SetViewData(const nsViewData& viewData) { m_ViewData = viewData; }
  NS_ALWAYS_INLINE const nsViewData& GetViewData() const { return m_ViewData; }

  NS_ALWAYS_INLINE void SetWorldTime(nsTime time) { m_WorldTime = time; }
  NS_ALWAYS_INLINE nsTime GetWorldTime() const { return m_WorldTime; }

  NS_ALWAYS_INLINE void SetWorldDebugContext(const nsDebugRendererContext& debugContext) { m_WorldDebugContext = debugContext; }
  NS_ALWAYS_INLINE const nsDebugRendererContext& GetWorldDebugContext() const { return m_WorldDebugContext; }

  NS_ALWAYS_INLINE void SetViewDebugContext(const nsDebugRendererContext& debugContext) { m_ViewDebugContext = debugContext; }
  NS_ALWAYS_INLINE const nsDebugRendererContext& GetViewDebugContext() const { return m_ViewDebugContext; }

  void AddRenderData(const nsRenderData* pRenderData, nsRenderData::Category category);
  void AddFrameData(const nsRenderData* pFrameData);

  void SortAndBatch();

  void Clear();

  nsRenderDataBatchList GetRenderDataBatchesWithCategory(
    nsRenderData::Category category, nsRenderDataBatch::Filter filter = nsRenderDataBatch::Filter()) const;

  template <typename T>
  NS_ALWAYS_INLINE const T* GetFrameData() const
  {
    return static_cast<const T*>(GetFrameData(nsGetStaticRTTI<T>()));
  }

private:
  const nsRenderData* GetFrameData(const nsRTTI* pRtti) const;

  struct DataPerCategory
  {
    nsDynamicArray<nsRenderDataBatch> m_Batches;
    nsDynamicArray<nsRenderDataBatch::SortableRenderData> m_SortableRenderData;
  };

  nsCamera m_Camera;
  nsCamera m_LodCamera; // Temporary until we have a real LOD system
  nsViewData m_ViewData;
  nsTime m_WorldTime;

  nsDebugRendererContext m_WorldDebugContext;
  nsDebugRendererContext m_ViewDebugContext;

  nsHybridArray<DataPerCategory, 16> m_DataPerCategory;
  nsHybridArray<const nsRenderData*, 16> m_FrameData;
};
