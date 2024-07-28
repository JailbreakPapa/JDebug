#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/Pipeline/Renderer.h>

struct nsPerLensFlareData;
class nsRenderDataBatch;
using nsShaderResourceHandle = nsTypedResourceHandle<class nsShaderResource>;

/// \brief Implements rendering of lens flares
class NS_RENDERERCORE_DLL nsLensFlareRenderer : public nsRenderer
{
  NS_ADD_DYNAMIC_REFLECTION(nsLensFlareRenderer, nsRenderer);
  NS_DISALLOW_COPY_AND_ASSIGN(nsLensFlareRenderer);

public:
  nsLensFlareRenderer();
  ~nsLensFlareRenderer();

  // nsRenderer implementation
  virtual void GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const override;
  virtual void GetSupportedRenderDataCategories(nsHybridArray<nsRenderData::Category, 8>& ref_categories) const override;
  virtual void RenderBatch(
    const nsRenderViewContext& renderContext, const nsRenderPipelinePass* pPass, const nsRenderDataBatch& batch) const override;

protected:
  nsGALBufferHandle CreateLensFlareDataBuffer(nsUInt32 uiBufferSize) const;
  void DeleteLensFlareDataBuffer(nsGALBufferHandle hBuffer) const;
  virtual void FillLensFlareData(const nsRenderDataBatch& batch) const;

  nsShaderResourceHandle m_hShader;
  mutable nsDynamicArray<nsPerLensFlareData, nsAlignedAllocatorWrapper> m_LensFlareData;
};
