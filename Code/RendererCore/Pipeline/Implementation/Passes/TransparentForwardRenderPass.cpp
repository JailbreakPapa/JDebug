#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/TransparentForwardRenderPass.h>
#include <RendererCore/RenderContext/RenderContext.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTransparentForwardRenderPass, 1, nsRTTIDefaultAllocator<nsTransparentForwardRenderPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ResolvedDepth", m_PinResolvedDepth),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsTransparentForwardRenderPass::nsTransparentForwardRenderPass(const char* szName)
  : nsForwardRenderPass(szName)
{
}

nsTransparentForwardRenderPass::~nsTransparentForwardRenderPass()
{
  if (!m_hSceneColorSamplerState.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroySamplerState(m_hSceneColorSamplerState);
    m_hSceneColorSamplerState.Invalidate();
  }
}

void nsTransparentForwardRenderPass::Execute(const nsRenderViewContext& renderViewContext,
  const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  auto pColorInput = inputs[m_PinColor.m_uiInputIndex];
  if (pColorInput == nullptr)
  {
    return;
  }

  CreateSamplerState();

  nsUInt32 uiWidth = pColorInput->m_Desc.m_uiWidth;
  nsUInt32 uiHeight = pColorInput->m_Desc.m_uiHeight;

  nsGALTextureCreationDescription desc;
  desc.SetAsRenderTarget(uiWidth, uiHeight, pColorInput->m_Desc.m_Format);
  desc.m_uiArraySize = pColorInput->m_Desc.m_uiArraySize;
  desc.m_uiMipLevelCount = 1;

  nsGALTextureHandle hSceneColor = nsGPUResourcePool::GetDefaultInstance()->GetRenderTarget(desc);

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsGALPass* pGALPass = pDevice->BeginPass(GetName());

  SetupResources(pGALPass, renderViewContext, inputs, outputs);
  SetupPermutationVars(renderViewContext);
  SetupLighting(renderViewContext);

  UpdateSceneColorTexture(renderViewContext, hSceneColor, pColorInput->m_TextureHandle);

  nsGALTextureResourceViewHandle colorResourceViewHandle = pDevice->GetDefaultResourceView(hSceneColor);
  renderViewContext.m_pRenderContext->BindTexture2D("SceneColor", colorResourceViewHandle);
  renderViewContext.m_pRenderContext->BindSamplerState("SceneColorSampler", m_hSceneColorSamplerState);

  RenderObjects(renderViewContext);

  renderViewContext.m_pRenderContext->EndRendering();
  pDevice->EndPass(pGALPass);

  nsGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hSceneColor);
}

void nsTransparentForwardRenderPass::SetupResources(nsGALPass* pGALPass, const nsRenderViewContext& renderViewContext,
  const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  SUPER::SetupResources(pGALPass, renderViewContext, inputs, outputs);

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  if (inputs[m_PinResolvedDepth.m_uiInputIndex])
  {
    nsGALTextureResourceViewHandle depthResourceViewHandle = pDevice->GetDefaultResourceView(inputs[m_PinResolvedDepth.m_uiInputIndex]->m_TextureHandle);
    renderViewContext.m_pRenderContext->BindTexture2D("SceneDepth", depthResourceViewHandle);
  }
}

void nsTransparentForwardRenderPass::RenderObjects(const nsRenderViewContext& renderViewContext)
{
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::LitTransparent);

  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("PREPARE_DEPTH", "TRUE");
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::LitForeground);

  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("PREPARE_DEPTH", "FALSE");
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::LitForeground);

  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::LitScreenFX);
}

void nsTransparentForwardRenderPass::UpdateSceneColorTexture(
  const nsRenderViewContext& renderViewContext, nsGALTextureHandle hSceneColorTexture, nsGALTextureHandle hCurrentColorTexture)
{
  nsGALTextureSubresource subresource;
  subresource.m_uiMipLevel = 0;
  subresource.m_uiArraySlice = 0;

  renderViewContext.m_pRenderContext->GetCommandEncoder()->ResolveTexture(hSceneColorTexture, subresource, hCurrentColorTexture, subresource);
}

void nsTransparentForwardRenderPass::CreateSamplerState()
{
  if (m_hSceneColorSamplerState.IsInvalidated())
  {
    nsGALSamplerStateCreationDescription desc;
    desc.m_MinFilter = nsGALTextureFilterMode::Linear;
    desc.m_MagFilter = nsGALTextureFilterMode::Linear;
    desc.m_MipFilter = nsGALTextureFilterMode::Linear;
    desc.m_AddressU = nsImageAddressMode::Clamp;
    desc.m_AddressV = nsImageAddressMode::Mirror;
    desc.m_AddressW = nsImageAddressMode::Mirror;

    m_hSceneColorSamplerState = nsGALDevice::GetDefaultDevice()->CreateSamplerState(desc);
  }
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_TransparentForwardRenderPass);
