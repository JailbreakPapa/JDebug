#pragma once

#include <RendererCore/Meshes/MeshRenderer.h>

/// \brief Implements rendering of skinned meshes
class NS_RENDERERCORE_DLL nsSkinnedMeshRenderer : public nsMeshRenderer
{
  NS_ADD_DYNAMIC_REFLECTION(nsSkinnedMeshRenderer, nsMeshRenderer);
  NS_DISALLOW_COPY_AND_ASSIGN(nsSkinnedMeshRenderer);

public:
  nsSkinnedMeshRenderer();
  ~nsSkinnedMeshRenderer();

  // nsRenderer implementation
  virtual void GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const override;

protected:
  virtual void SetAdditionalData(const nsRenderViewContext& renderViewContext, const nsMeshRenderData* pRenderData) const override;

  static nsUInt32 s_uiSkinningBufferUpdates;
};
