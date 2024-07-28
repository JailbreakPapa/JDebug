#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/Pipeline/Passes/SimpleRenderPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

#include <RendererCore/Debug/DebugRenderer.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSimpleRenderPass, 1, nsRTTIDefaultAllocator<nsSimpleRenderPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_PinColor),
    NS_MEMBER_PROPERTY("DepthStencil", m_PinDepthStencil),
    NS_MEMBER_PROPERTY("Message", m_sMessage),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSimpleRenderPass::nsSimpleRenderPass(const char* szName)
  : nsRenderPipelinePass(szName, true)
{
}

nsSimpleRenderPass::~nsSimpleRenderPass() = default;

bool nsSimpleRenderPass::GetRenderTargetDescriptions(
  const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  const nsGALRenderTargets& renderTargets = view.GetActiveRenderTargets();

  // Color
  if (inputs[m_PinColor.m_uiInputIndex])
  {
    outputs[m_PinColor.m_uiOutputIndex] = *inputs[m_PinColor.m_uiInputIndex];
  }
  else
  {
    // If no input is available, we use the render target setup instead.
    const nsGALTexture* pTexture = pDevice->GetTexture(renderTargets.m_hRTs[0]);
    if (pTexture)
    {
      outputs[m_PinColor.m_uiOutputIndex] = pTexture->GetDescription();
      outputs[m_PinColor.m_uiOutputIndex].m_bCreateRenderTarget = true;
      outputs[m_PinColor.m_uiOutputIndex].m_bAllowShaderResourceView = true;
      outputs[m_PinColor.m_uiOutputIndex].m_ResourceAccess.m_bReadBack = false;
      outputs[m_PinColor.m_uiOutputIndex].m_ResourceAccess.m_bImmutable = true;
      outputs[m_PinColor.m_uiOutputIndex].m_pExisitingNativeObject = nullptr;
    }
  }

  // DepthStencil
  if (inputs[m_PinDepthStencil.m_uiInputIndex])
  {
    outputs[m_PinDepthStencil.m_uiOutputIndex] = *inputs[m_PinDepthStencil.m_uiInputIndex];
  }
  else
  {
    // If no input is available, we use the render target setup instead.
    const nsGALTexture* pTexture = pDevice->GetTexture(renderTargets.m_hDSTarget);
    if (pTexture)
    {
      outputs[m_PinDepthStencil.m_uiOutputIndex] = pTexture->GetDescription();
    }
  }

  return true;
}

void nsSimpleRenderPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs,
  const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
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

  auto pCommandEncoder = nsRenderContext::BeginPassAndRenderingScope(renderViewContext, std::move(renderingSetup), GetName(), renderViewContext.m_pCamera->IsStereoscopic());

  // Setup Permutation Vars
  nsTempHashedString sRenderPass("RENDER_PASS_FORWARD");
  if (renderViewContext.m_pViewData->m_ViewRenderMode != nsViewRenderMode::None)
  {
    sRenderPass = nsViewRenderMode::GetPermutationValue(renderViewContext.m_pViewData->m_ViewRenderMode);
  }

  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("RENDER_PASS", sRenderPass);

  // Execute render functions
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::SimpleOpaque);
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::SimpleTransparent);

  if (!m_sMessage.IsEmpty())
  {
    nsDebugRenderer::Draw2DText(*renderViewContext.m_pViewDebugContext, m_sMessage.GetData(), nsVec2I32(20, 20), nsColor::OrangeRed);
  }

  nsDebugRenderer::RenderWorldSpace(renderViewContext);

  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("PREPARE_DEPTH", "TRUE");
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::SimpleForeground);

  renderViewContext.m_pRenderContext->SetShaderPermutationVariable("PREPARE_DEPTH", "FALSE");
  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::SimpleForeground);

  RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::GUI);

  nsDebugRenderer::RenderScreenSpace(renderViewContext);
}

nsResult nsSimpleRenderPass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_sMessage;
  return NS_SUCCESS;
}

nsResult nsSimpleRenderPass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_sMessage;
  return NS_SUCCESS;
}

void nsSimpleRenderPass::SetMessage(const char* szMessage)
{
  m_sMessage = szMessage;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_SimpleRenderPass);
