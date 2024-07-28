#pragma once

#include <RendererCore/Pipeline/Renderer.h>

class nsMeshRenderData;
struct nsPerInstanceData;

/// \brief Implements rendering of static meshes
class NS_RENDERERCORE_DLL nsMeshRenderer : public nsRenderer
{
  NS_ADD_DYNAMIC_REFLECTION(nsMeshRenderer, nsRenderer);
  NS_DISALLOW_COPY_AND_ASSIGN(nsMeshRenderer);

public:
  nsMeshRenderer();
  ~nsMeshRenderer();

  // nsRenderer implementation
  virtual void GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const override;
  virtual void GetSupportedRenderDataCategories(nsHybridArray<nsRenderData::Category, 8>& ref_categories) const override;
  virtual void RenderBatch(
    const nsRenderViewContext& renderContext, const nsRenderPipelinePass* pPass, const nsRenderDataBatch& batch) const override;

protected:
  virtual void SetAdditionalData(const nsRenderViewContext& renderViewContext, const nsMeshRenderData* pRenderData) const;
  virtual void FillPerInstanceData(
    nsArrayPtr<nsPerInstanceData> instanceData, const nsRenderDataBatch& batch, nsUInt32 uiStartIndex, nsUInt32& out_uiFilteredCount) const;
};
