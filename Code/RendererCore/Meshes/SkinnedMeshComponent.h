#pragma once

#include <RendererCore/Meshes/MeshComponentBase.h>
#include <RendererFoundation/Shader/Types.h>
#include <memory>

class nsShaderTransform;

class NS_RENDERERCORE_DLL nsSkinnedMeshRenderData : public nsMeshRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsSkinnedMeshRenderData, nsMeshRenderData);

public:
  virtual void FillBatchIdAndSortingKey() override;
  nsGALBufferHandle m_hSkinningTransforms;
  nsArrayPtr<const nsUInt8> m_pNewSkinningTransformData;
  std::shared_ptr<bool> m_bTransformsUpdated;
};

struct NS_RENDERERCORE_DLL nsSkinningState
{
  nsSkinningState();
  ~nsSkinningState();

  void Clear();

  /// \brief Holds the current CPU-side copy of the skinning matrices. Modify these and call TransformsChanged() to send them to the GPU.
  nsDynamicArray<nsShaderTransform, nsAlignedAllocatorWrapper> m_Transforms;

  /// \brief Call this, after modifying m_Transforms, to make the renderer apply the update.
  void TransformsChanged();

  void FillSkinnedMeshRenderData(nsSkinnedMeshRenderData& ref_renderData) const;

private:
  nsGALBufferHandle m_hGpuBuffer;
  std::shared_ptr<bool> m_bTransformsUpdated[2];
};
