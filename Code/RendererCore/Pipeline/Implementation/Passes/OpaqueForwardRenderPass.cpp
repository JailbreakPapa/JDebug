#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/Passes/OpaqueForwardRenderPass.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/Texture2DResource.h>

#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsOpaqueForwardRenderPass, 1, nsRTTIDefaultAllocator<nsOpaqueForwardRenderPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("SSAO", m_PinSSAO),
    NS_MEMBER_PROPERTY("WriteDepth", m_bWriteDepth)->AddAttributes(new nsDefaultValueAttribute(true)),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsOpaqueForwardRenderPass::nsOpaqueForwardRenderPass(const char* szName)
  : nsForwardRenderPass(szName)

{
  m_hWhiteTexture = nsResourceManager::LoadResource<nsTexture2DResource>("White.color");
}

nsOpaqueForwardRenderPass::~nsOpaqueForwardRenderPass() = default;

bool nsOpaqueForwardRenderPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  if (!SUPER::GetRenderTargetDescriptions(view, inputs, outputs))
  {
    return false;
  }

  if (inputs[m_PinSSAO.m_uiInputIndex])
  {
    if (inputs[m_PinSSAO.m_uiInputIndex]->m_uiWidth != inputs[m_PinColor.m_uiInputIndex]->m_uiWidth ||
        inputs[m_PinSSAO.m_uiInputIndex]->m_uiHeight != inputs[m_PinColor.m_uiInputIndex]->m_uiHeight)
    {
      nsLog::Warning("Expected same resolution for SSAO and color input to pass '{0}'!", GetName());
    }

    if (m_ShadingQuality == nsForwardRenderShadingQuality::Simplified)
    {
      nsLog::Warning("SSAO input will be ignored for pass '{0}' since simplified shading is activated.", GetName());
    }
  }

  return true;
}

void nsOpaqueForwardRenderPass::SetupResources(nsGALPass* pGALPass, const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  SUPER::SetupResources(pGALPass, renderViewContext, inputs, outputs);

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  // SSAO texture
  if (m_ShadingQuality == nsForwardRenderShadingQuality::Normal)
  {
    if (inputs[m_PinSSAO.m_uiInputIndex])
    {
      nsGALTextureResourceViewHandle ssaoResourceViewHandle = pDevice->GetDefaultResourceView(inputs[m_PinSSAO.m_uiInputIndex]->m_TextureHandle);
      renderViewContext.m_pRenderContext->BindTexture2D("SSAOTexture", ssaoResourceViewHandle);
    }
    else
    {
      renderViewContext.m_pRenderContext->BindTexture2D("SSAOTexture", m_hWhiteTexture, nsResourceAcquireMode::BlockTillLoaded);
    }
  }
}

void nsOpaqueForwardRenderPass::SetupPermutationVars(const nsRenderViewContext& renderViewContext)
{
  SUPER::SetupPermutationVars(renderViewContext);

  if (m_bWriteDepth)
  {
    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("FORWARD_PASS_WRITE_DEPTH", "TRUE");
  }
  else
  {
    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("FORWARD_PASS_WRITE_DEPTH", "FALSE");
  }
}

void nsOpaqueForwardRenderPass::RenderObjects(const nsRenderViewContext& renderViewContext)
{
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::LitOpaque);
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::LitMasked);
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_OpaqueForwardRenderPass);
