#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Lights/ClusteredDataProvider.h>
#include <RendererCore/Lights/SimplifiedDataProvider.h>
#include <RendererCore/Pipeline/Passes/ForwardRenderPass.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/Texture2DResource.h>

#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsForwardRenderPass, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_PinColor),
    NS_MEMBER_PROPERTY("DepthStencil", m_PinDepthStencil),
    NS_ENUM_MEMBER_PROPERTY("ShadingQuality", nsForwardRenderShadingQuality, m_ShadingQuality)->AddAttributes(new nsDefaultValueAttribute((int)nsForwardRenderShadingQuality::Normal)),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsForwardRenderShadingQuality, 1)
  NS_ENUM_CONSTANTS(nsForwardRenderShadingQuality::Normal, nsForwardRenderShadingQuality::Simplified)
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

nsForwardRenderPass::nsForwardRenderPass(const char* szName)
  : nsRenderPipelinePass(szName, true)
  , m_ShadingQuality(nsForwardRenderShadingQuality::Normal)
{
}

nsForwardRenderPass::~nsForwardRenderPass() = default;

bool nsForwardRenderPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinColor.m_uiInputIndex])
  {
    outputs[m_PinColor.m_uiOutputIndex] = *inputs[m_PinColor.m_uiInputIndex];
  }
  else
  {
    nsLog::Error("No color input connected to pass '{0}'!", GetName());
    return false;
  }

  // DepthStencil
  if (inputs[m_PinDepthStencil.m_uiInputIndex])
  {
    outputs[m_PinDepthStencil.m_uiOutputIndex] = *inputs[m_PinDepthStencil.m_uiInputIndex];
  }
  else
  {
    nsLog::Error("No depth stencil input connected to pass '{0}'!", GetName());
    return false;
  }

  return true;
}

void nsForwardRenderPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  nsGALPass* pGALPass = pDevice->BeginPass(GetName());

  SetupResources(pGALPass, renderViewContext, inputs, outputs);
  SetupPermutationVars(renderViewContext);
  SetupLighting(renderViewContext);

  RenderObjects(renderViewContext);

  renderViewContext.m_pRenderContext->EndRendering();
  pDevice->EndPass(pGALPass);
}

nsResult nsForwardRenderPass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_ShadingQuality;
  return NS_SUCCESS;
}

nsResult nsForwardRenderPass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_ShadingQuality;
  return NS_SUCCESS;
}

void nsForwardRenderPass::SetupResources(nsGALPass* pGALPass, const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  // Setup render target
  nsGALRenderingSetup renderingSetup;
  if (inputs[m_PinColor.m_uiInputIndex])
  {
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(inputs[m_PinColor.m_uiInputIndex]->m_TextureHandle));
  }

  if (inputs[m_PinDepthStencil.m_uiInputIndex])
  {
    renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(pDevice->GetDefaultRenderTargetView(inputs[m_PinDepthStencil.m_uiInputIndex]->m_TextureHandle));
  }

  renderViewContext.m_pRenderContext->BeginRendering(pGALPass, std::move(renderingSetup), renderViewContext.m_pViewData->m_ViewPortRect, "", renderViewContext.m_pCamera->IsStereoscopic());
}

void nsForwardRenderPass::SetupPermutationVars(const nsRenderViewContext& renderViewContext)
{
  nsTempHashedString sRenderPass("RENDER_PASS_FORWARD");
  if (renderViewContext.m_pViewData->m_ViewRenderMode != nsViewRenderMode::None)
  {
    sRenderPass = nsViewRenderMode::GetPermutationValue(renderViewContext.m_pViewData->m_ViewRenderMode);
  }

  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("RENDER_PASS", sRenderPass);

  nsStringBuilder sDebugText;
  nsViewRenderMode::GetDebugText(renderViewContext.m_pViewData->m_ViewRenderMode, sDebugText);
  if (!sDebugText.IsEmpty())
  {
    nsDebugRenderer::Draw2DText(*renderViewContext.m_pViewDebugContext, sDebugText, nsVec2I32(10, 10), nsColor::White);
  }

  // Set permutation for shading quality
  if (m_ShadingQuality == nsForwardRenderShadingQuality::Normal)
  {
    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("SHADING_QUALITY", "SHADING_QUALITY_NORMAL");
  }
  else if (m_ShadingQuality == nsForwardRenderShadingQuality::Simplified)
  {
    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("SHADING_QUALITY", "SHADING_QUALITY_SIMPLIFIED");
  }
  else
  {
    NS_REPORT_FAILURE("Unknown shading quality setting.");
  }
}

void nsForwardRenderPass::SetupLighting(const nsRenderViewContext& renderViewContext)
{
  // Setup clustered data
  if (m_ShadingQuality == nsForwardRenderShadingQuality::Normal)
  {
    auto pClusteredData = GetPipeline()->GetFrameDataProvider<nsClusteredDataProvider>()->GetData(renderViewContext);
    pClusteredData->BindResources(renderViewContext.m_pRenderContext);
  }
  // Or other light properties.
  else
  {
    auto pSimplifiedData = GetPipeline()->GetFrameDataProvider<nsSimplifiedDataProvider>()->GetData(renderViewContext);
    pSimplifiedData->BindResources(renderViewContext.m_pRenderContext);
    // todo
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_ForwardRenderPass);
