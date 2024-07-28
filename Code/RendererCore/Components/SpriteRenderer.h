#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/Pipeline/Renderer.h>

struct nsPerSpriteData;
class nsRenderDataBatch;
using nsShaderResourceHandle = nsTypedResourceHandle<class nsShaderResource>;

/// \brief Implements rendering of sprites
class NS_RENDERERCORE_DLL nsSpriteRenderer : public nsRenderer
{
  NS_ADD_DYNAMIC_REFLECTION(nsSpriteRenderer, nsRenderer);
  NS_DISALLOW_COPY_AND_ASSIGN(nsSpriteRenderer);

public:
  nsSpriteRenderer();
  ~nsSpriteRenderer();

  // nsRenderer implementation
  virtual void GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const override;
  virtual void GetSupportedRenderDataCategories(nsHybridArray<nsRenderData::Category, 8>& ref_categories) const override;
  virtual void RenderBatch(
    const nsRenderViewContext& renderContext, const nsRenderPipelinePass* pPass, const nsRenderDataBatch& batch) const override;

protected:
  nsGALBufferHandle CreateSpriteDataBuffer(nsUInt32 uiBufferSize) const;
  void DeleteSpriteDataBuffer(nsGALBufferHandle hBuffer) const;
  virtual void FillSpriteData(const nsRenderDataBatch& batch) const;

  nsShaderResourceHandle m_hShader;
  mutable nsDynamicArray<nsPerSpriteData, nsAlignedAllocatorWrapper> m_SpriteData;
};
