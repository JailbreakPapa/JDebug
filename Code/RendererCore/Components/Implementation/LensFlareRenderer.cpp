#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Math/Float16.h>
#include <Foundation/Types/ScopeExit.h>
#include <RendererCore/Components/LensFlareComponent.h>
#include <RendererCore/Components/LensFlareRenderer.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/Shader/ShaderUtils.h>

#include <Shaders/Materials/LensFlareData.h>
NS_CHECK_AT_COMPILETIME(sizeof(nsPerLensFlareData) == 48);

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLensFlareRenderer, 1, nsRTTIDefaultAllocator<nsLensFlareRenderer>)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsLensFlareRenderer::nsLensFlareRenderer()
{
  m_hShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Materials/LensFlareMaterial.nsShader");
}

nsLensFlareRenderer::~nsLensFlareRenderer() = default;

void nsLensFlareRenderer::GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const
{
  ref_types.PushBack(nsGetStaticRTTI<nsLensFlareRenderData>());
}

void nsLensFlareRenderer::GetSupportedRenderDataCategories(nsHybridArray<nsRenderData::Category, 8>& ref_categories) const
{
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitTransparent);
}

void nsLensFlareRenderer::RenderBatch(const nsRenderViewContext& renderViewContext, const nsRenderPipelinePass* pPass, const nsRenderDataBatch& batch) const
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsRenderContext* pContext = renderViewContext.m_pRenderContext;

  const nsLensFlareRenderData* pRenderData = batch.GetFirstData<nsLensFlareRenderData>();

  const nsUInt32 uiBufferSize = nsMath::RoundUp(batch.GetCount(), 128u);
  nsGALBufferHandle hLensFlareData = CreateLensFlareDataBuffer(uiBufferSize);
  NS_SCOPE_EXIT(DeleteLensFlareDataBuffer(hLensFlareData));

  pContext->BindShader(m_hShader);
  pContext->BindBuffer("lensFlareData", pDevice->GetDefaultResourceView(hLensFlareData));
  pContext->BindTexture2D("LensFlareTexture", pRenderData->m_hTexture);

  FillLensFlareData(batch);

  if (m_LensFlareData.GetCount() > 0) // Instance data might be empty if all render data was filtered.
  {
    pContext->GetCommandEncoder()->UpdateBuffer(hLensFlareData, 0, m_LensFlareData.GetByteArrayPtr());

    pContext->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, m_LensFlareData.GetCount() * 2);
    pContext->DrawMeshBuffer().IgnoreResult();
  }
}

nsGALBufferHandle nsLensFlareRenderer::CreateLensFlareDataBuffer(nsUInt32 uiBufferSize) const
{
  nsGALBufferCreationDescription desc;
  desc.m_uiStructSize = sizeof(nsPerLensFlareData);
  desc.m_uiTotalSize = desc.m_uiStructSize * uiBufferSize;
  desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
  desc.m_ResourceAccess.m_bImmutable = false;

  return nsGPUResourcePool::GetDefaultInstance()->GetBuffer(desc);
}

void nsLensFlareRenderer::DeleteLensFlareDataBuffer(nsGALBufferHandle hBuffer) const
{
  nsGPUResourcePool::GetDefaultInstance()->ReturnBuffer(hBuffer);
}

void nsLensFlareRenderer::FillLensFlareData(const nsRenderDataBatch& batch) const
{
  m_LensFlareData.Clear();
  m_LensFlareData.Reserve(batch.GetCount());

  for (auto it = batch.GetIterator<nsLensFlareRenderData>(); it.IsValid(); ++it)
  {
    const nsLensFlareRenderData* pRenderData = it;

    auto& LensFlareData = m_LensFlareData.ExpandAndGetRef();
    LensFlareData.WorldSpacePosition = pRenderData->m_GlobalTransform.m_vPosition;
    LensFlareData.Size = pRenderData->m_fSize;
    LensFlareData.MaxScreenSize = pRenderData->m_fMaxScreenSize;
    LensFlareData.OcclusionRadius = pRenderData->m_fOcclusionSampleRadius;
    LensFlareData.OcclusionSpread = pRenderData->m_fOcclusionSampleSpread;
    LensFlareData.DepthOffset = pRenderData->m_fOcclusionDepthOffset;
    LensFlareData.AspectRatioAndShift = nsShaderUtils::Float2ToRG16F(nsVec2(pRenderData->m_fAspectRatio, pRenderData->m_fShiftToCenter));
    LensFlareData.ColorRG = nsShaderUtils::PackFloat16intoUint(pRenderData->m_Color.x, pRenderData->m_Color.y);
    LensFlareData.ColorBA = nsShaderUtils::PackFloat16intoUint(pRenderData->m_Color.z, pRenderData->m_Color.w);
    LensFlareData.Flags = (pRenderData->m_bInverseTonemap ? LENS_FLARE_INVERSE_TONEMAP : 0) |
                          (pRenderData->m_bGreyscaleTexture ? LENS_FLARE_GREYSCALE_TEXTURE : 0) |
                          (pRenderData->m_bApplyFog ? LENS_FLARE_APPLY_FOG : 0);
  }
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_LensFlareRenderer);
