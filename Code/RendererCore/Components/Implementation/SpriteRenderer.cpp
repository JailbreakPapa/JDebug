#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Math/Float16.h>
#include <Foundation/Types/ScopeExit.h>
#include <RendererCore/Components/SpriteComponent.h>
#include <RendererCore/Components/SpriteRenderer.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/RenderDataBatch.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Shader/ShaderResource.h>
#include <RendererFoundation/Shader/ShaderUtils.h>

#include <Shaders/Materials/SpriteData.h>
NS_CHECK_AT_COMPILETIME(sizeof(nsPerSpriteData) == 48);

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSpriteRenderer, 1, nsRTTIDefaultAllocator<nsSpriteRenderer>)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsSpriteRenderer::nsSpriteRenderer()
{
  m_hShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Materials/SpriteMaterial.nsShader");
}

nsSpriteRenderer::~nsSpriteRenderer() = default;

void nsSpriteRenderer::GetSupportedRenderDataTypes(nsHybridArray<const nsRTTI*, 8>& ref_types) const
{
  ref_types.PushBack(nsGetStaticRTTI<nsSpriteRenderData>());
}

void nsSpriteRenderer::GetSupportedRenderDataCategories(nsHybridArray<nsRenderData::Category, 8>& ref_categories) const
{
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitMasked);
  ref_categories.PushBack(nsDefaultRenderDataCategories::LitTransparent);
  ref_categories.PushBack(nsDefaultRenderDataCategories::SimpleOpaque);
  ref_categories.PushBack(nsDefaultRenderDataCategories::SimpleTransparent);
  ref_categories.PushBack(nsDefaultRenderDataCategories::Selection);
}

void nsSpriteRenderer::RenderBatch(const nsRenderViewContext& renderViewContext, const nsRenderPipelinePass* pPass, const nsRenderDataBatch& batch) const
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsRenderContext* pContext = renderViewContext.m_pRenderContext;

  const nsSpriteRenderData* pRenderData = batch.GetFirstData<nsSpriteRenderData>();

  const nsUInt32 uiBufferSize = nsMath::RoundUp(batch.GetCount(), 128u);
  nsGALBufferHandle hSpriteData = CreateSpriteDataBuffer(uiBufferSize);
  NS_SCOPE_EXIT(DeleteSpriteDataBuffer(hSpriteData));

  pContext->BindShader(m_hShader);
  pContext->BindBuffer("spriteData", pDevice->GetDefaultResourceView(hSpriteData));
  pContext->BindTexture2D("SpriteTexture", pRenderData->m_hTexture);

  pContext->SetShaderPermutationVariable("BLEND_MODE", nsSpriteBlendMode::GetPermutationValue(pRenderData->m_BlendMode));
  pContext->SetShaderPermutationVariable("SHAPE_ICON", pRenderData->m_BlendMode == nsSpriteBlendMode::ShapeIcon ? nsMakeHashedString("TRUE") : nsMakeHashedString("FALSE"));

  FillSpriteData(batch);

  if (m_SpriteData.GetCount() > 0) // Instance data might be empty if all render data was filtered.
  {
    pContext->GetCommandEncoder()->UpdateBuffer(hSpriteData, 0, m_SpriteData.GetByteArrayPtr());

    pContext->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, m_SpriteData.GetCount() * 2);
    pContext->DrawMeshBuffer().IgnoreResult();
  }
}

nsGALBufferHandle nsSpriteRenderer::CreateSpriteDataBuffer(nsUInt32 uiBufferSize) const
{
  nsGALBufferCreationDescription desc;
  desc.m_uiStructSize = sizeof(nsPerSpriteData);
  desc.m_uiTotalSize = desc.m_uiStructSize * uiBufferSize;
  desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
  desc.m_ResourceAccess.m_bImmutable = false;

  return nsGPUResourcePool::GetDefaultInstance()->GetBuffer(desc);
}

void nsSpriteRenderer::DeleteSpriteDataBuffer(nsGALBufferHandle hBuffer) const
{
  nsGPUResourcePool::GetDefaultInstance()->ReturnBuffer(hBuffer);
}

void nsSpriteRenderer::FillSpriteData(const nsRenderDataBatch& batch) const
{
  m_SpriteData.Clear();
  m_SpriteData.Reserve(batch.GetCount());

  for (auto it = batch.GetIterator<nsSpriteRenderData>(); it.IsValid(); ++it)
  {
    const nsSpriteRenderData* pRenderData = it;

    auto& spriteData = m_SpriteData.ExpandAndGetRef();

    spriteData.WorldSpacePosition = pRenderData->m_GlobalTransform.m_vPosition;
    spriteData.Size = pRenderData->m_fSize;
    spriteData.MaxScreenSize = pRenderData->m_fMaxScreenSize;
    spriteData.AspectRatio = pRenderData->m_fAspectRatio;
    spriteData.ColorRG = nsShaderUtils::Float2ToRG16F(nsVec2(pRenderData->m_color.r, pRenderData->m_color.g));
    spriteData.ColorBA = nsShaderUtils::Float2ToRG16F(nsVec2(pRenderData->m_color.b, pRenderData->m_color.a));
    spriteData.TexCoordScale = nsShaderUtils::Float2ToRG16F(pRenderData->m_texCoordScale);
    spriteData.TexCoordOffset = nsShaderUtils::Float2ToRG16F(pRenderData->m_texCoordOffset);
    spriteData.GameObjectID = pRenderData->m_uiUniqueID;
    spriteData.Reserved = 0;
  }
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_SpriteRenderer);
