#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Meshes/SkinnedMeshComponent.h>
#include <RendererCore/Meshes/SkinnedMeshRenderer.h>
#include <RendererCore/RenderContext/RenderContext.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSkinnedMeshRenderer, 1, nsRTTIDefaultAllocator<nsSkinnedMeshRenderer>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsUInt32 nsSkinnedMeshRenderer::s_uiSkinningBufferUpdates = 0;

nsSkinnedMeshRenderer::nsSkinnedMeshRenderer() = default;
nsSkinnedMeshRenderer::~nsSkinnedMeshRenderer() = default;

void nsSkinnedMeshRenderer::GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const
{
  ref_types.PushBack(nsGetStaticRTTI<nsSkinnedMeshRenderData>());
}

void nsSkinnedMeshRenderer::SetAdditionalData(const nsRenderViewContext& renderViewContext, const nsMeshRenderData* pRenderData) const
{
  // Don't call base class implementation here since the state will be overwritten in this method anyways.

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsRenderContext* pContext = renderViewContext.m_pRenderContext;

  auto pSkinnedRenderData = static_cast<const nsSkinnedMeshRenderData*>(pRenderData);

  if (pSkinnedRenderData->m_hSkinningTransforms.IsInvalidated())
  {
    pContext->SetShaderPermutationVariable("VERTEX_SKINNING", "FALSE");
  }
  else
  {
    pContext->SetShaderPermutationVariable("VERTEX_SKINNING", "TRUE");

    if (pSkinnedRenderData->m_bTransformsUpdated != nullptr && *pSkinnedRenderData->m_bTransformsUpdated == false)
    {
      // if this is the first renderer that is supposed to actually render the skinned mesh, upload the skinning matrices
      *pSkinnedRenderData->m_bTransformsUpdated = true;
      pContext->GetCommandEncoder()->UpdateBuffer(pSkinnedRenderData->m_hSkinningTransforms, 0, pSkinnedRenderData->m_pNewSkinningTransformData);

      // TODO: could expose this somewhere (nsStats?)
      s_uiSkinningBufferUpdates++;
    }

    pContext->BindBuffer("skinningTransforms", pDevice->GetDefaultResourceView(pSkinnedRenderData->m_hSkinningTransforms));
  }
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_SkinnedMeshRenderer);
