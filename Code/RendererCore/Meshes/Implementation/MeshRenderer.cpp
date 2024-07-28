#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/Implementation/MeshRendererUtils.h>
#include <RendererCore/Meshes/InstancedMeshComponent.h>
#include <RendererCore/Meshes/MeshRenderer.h>
#include <RendererCore/Pipeline/InstanceDataProvider.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/RenderPipelinePass.h>
#include <RendererCore/RenderContext/RenderContext.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMeshRenderer, 1, nsRTTIDefaultAllocator<nsMeshRenderer>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsMeshRenderer::nsMeshRenderer() = default;
nsMeshRenderer::~nsMeshRenderer() = default;

void nsMeshRenderer::GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const
{
  ref_types.PushBack(nsGetStaticRTTI<nsMeshRenderData>());
  ref_types.PushBack(nsGetStaticRTTI<nsInstancedMeshRenderData>());
}

void nsMeshRenderer::GetSupportedRenderDataCategories(nsHybridArray<nsRenderData::Category, 8>& ref_categories) const
{
  ref_categories.PushBack(nsDefaultRenderDataCategories::Sky);
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitOpaque);
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitMasked);
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitTransparent);
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitForeground);
  ref_categories.PushBack(nsDefaultRenderDataCategories::SimpleOpaque);
  ref_categories.PushBack(nsDefaultRenderDataCategories::SimpleTransparent);
  ref_categories.PushBack(nsDefaultRenderDataCategories::SimpleForeground);
  ref_categories.PushBack(nsDefaultRenderDataCategories::Selection);
  ref_categories.PushBack(nsDefaultRenderDataCategories::GUI);
}

void nsMeshRenderer::RenderBatch(const nsRenderViewContext& renderViewContext, const nsRenderPipelinePass* pPass, const nsRenderDataBatch& batch) const
{
  nsRenderContext* pContext = renderViewContext.m_pRenderContext;

  const nsMeshRenderData* pRenderData = batch.GetFirstData<nsMeshRenderData>();

  const nsMeshResourceHandle& hMesh = pRenderData->m_hMesh;
  const nsMaterialResourceHandle& hMaterial = pRenderData->m_hMaterial;
  const nsUInt32 uiPartIndex = pRenderData->m_uiSubMeshIndex;
  const bool bHasExplicitInstanceData = pRenderData->IsInstanceOf<nsInstancedMeshRenderData>();

  nsResourceLock<nsMeshResource> pMesh(hMesh, nsResourceAcquireMode::AllowLoadingFallback);

  // This can happen when the resource has been reloaded and now has fewer submeshes.
  const auto& subMeshes = pMesh->GetSubMeshes();
  if (subMeshes.GetCount() <= uiPartIndex)
  {
    return;
  }

  nsInstanceData* pInstanceData = bHasExplicitInstanceData ? static_cast<const nsInstancedMeshRenderData*>(pRenderData)->m_pExplicitInstanceData : pPass->GetPipeline()->GetFrameDataProvider<nsInstanceDataProvider>()->GetData(renderViewContext);

  pInstanceData->BindResources(pContext);

  if (pRenderData->m_uiFlipWinding)
  {
    pContext->SetShaderPermutationVariable("FLIP_WINDING", "TRUE");
  }
  else
  {
    pContext->SetShaderPermutationVariable("FLIP_WINDING", "FALSE");
  }

  pContext->BindMaterial(hMaterial);
  pContext->BindMeshBuffer(pMesh->GetMeshBuffer());

  SetAdditionalData(renderViewContext, pRenderData);

  if (!bHasExplicitInstanceData)
  {
    nsUInt32 uiStartIndex = 0;
    while (uiStartIndex < batch.GetCount())
    {
      const nsUInt32 uiRemainingInstances = batch.GetCount() - uiStartIndex;

      nsUInt32 uiInstanceDataOffset = 0;
      nsArrayPtr<nsPerInstanceData> instanceData = pInstanceData->GetInstanceData(uiRemainingInstances, uiInstanceDataOffset);

      nsUInt32 uiFilteredCount = 0;
      FillPerInstanceData(instanceData, batch, uiStartIndex, uiFilteredCount);

      if (uiFilteredCount > 0) // Instance data might be empty if all render data was filtered.
      {
        pInstanceData->UpdateInstanceData(pContext, uiFilteredCount);

        const nsMeshResourceDescriptor::SubMesh& meshPart = subMeshes[uiPartIndex];

        if (pContext->DrawMeshBuffer(meshPart.m_uiPrimitiveCount, meshPart.m_uiFirstPrimitive, uiFilteredCount).Failed())
        {
          for (auto it = batch.GetIterator<nsMeshRenderData>(uiStartIndex, instanceData.GetCount()); it.IsValid(); ++it)
          {
            pRenderData = it;

            // draw bounding box instead
            if (pRenderData->m_GlobalBounds.IsValid())
            {
              nsDebugRenderer::DrawLineBox(*renderViewContext.m_pViewDebugContext, pRenderData->m_GlobalBounds.GetBox(), nsColor::Magenta);
            }
          }
        }
      }

      uiStartIndex += instanceData.GetCount();
    }
  }
  else
  {
    nsUInt32 uiInstanceCount = static_cast<const nsInstancedMeshRenderData*>(pRenderData)->m_uiExplicitInstanceCount;

    const nsMeshResourceDescriptor::SubMesh& meshPart = subMeshes[uiPartIndex];

    pContext->DrawMeshBuffer(meshPart.m_uiPrimitiveCount, meshPart.m_uiFirstPrimitive, uiInstanceCount).IgnoreResult();
  }
}

void nsMeshRenderer::SetAdditionalData(const nsRenderViewContext& renderViewContext, const nsMeshRenderData* pRenderData) const
{
  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("VERTEX_SKINNING", "FALSE");
}

void nsMeshRenderer::FillPerInstanceData(nsArrayPtr<nsPerInstanceData> instanceData, const nsRenderDataBatch& batch, nsUInt32 uiStartIndex, nsUInt32& out_uiFilteredCount) const
{
  nsUInt32 uiCount = nsMath::Min<nsUInt32>(instanceData.GetCount(), batch.GetCount() - uiStartIndex);
  nsUInt32 uiCurrentIndex = 0;

  for (auto it = batch.GetIterator<nsMeshRenderData>(uiStartIndex, uiCount); it.IsValid(); ++it)
  {
    nsInternal::FillPerInstanceData(instanceData[uiCurrentIndex], it);

    ++uiCurrentIndex;
  }

  out_uiFilteredCount = uiCurrentIndex;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_MeshRenderer);
