#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/SelectionHighlightPass.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Resources/RenderTargetSetup.h>
#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/SelectionHighlightConstants.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSelectionHighlightPass, 1, nsRTTIDefaultAllocator<nsSelectionHighlightPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_PinColor),
    NS_MEMBER_PROPERTY("DepthStencil", m_PinDepthStencil),

    NS_MEMBER_PROPERTY("HighlightColor", m_HighlightColor)->AddAttributes(new nsDefaultValueAttribute(nsColorScheme::LightUI(nsColorScheme::Yellow))),
    NS_MEMBER_PROPERTY("OverlayOpacity", m_fOverlayOpacity)->AddAttributes(new nsDefaultValueAttribute(0.1f))
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSelectionHighlightPass::nsSelectionHighlightPass(const char* szName)
  : nsRenderPipelinePass(szName, true)
{
  // Load shader.
  m_hShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/SelectionHighlight.nsShader");
  NS_ASSERT_DEV(m_hShader.IsValid(), "Could not load selection highlight shader!");

  m_hConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsSelectionHighlightConstants>();
}

nsSelectionHighlightPass::~nsSelectionHighlightPass()
{
  nsRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool nsSelectionHighlightPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinColor.m_uiInputIndex])
  {
    outputs[m_PinColor.m_uiOutputIndex] = *inputs[m_PinColor.m_uiInputIndex];
    return true;
  }

  return false;
}

void nsSelectionHighlightPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  auto pColorOutput = outputs[m_PinColor.m_uiOutputIndex];
  if (pColorOutput == nullptr)
  {
    return;
  }

  auto pDepthInput = inputs[m_PinDepthStencil.m_uiInputIndex];
  if (pDepthInput == nullptr)
  {
    return;
  }

  nsRenderDataBatchList renderDataBatchList = GetPipeline()->GetRenderDataBatchesWithCategory(nsDefaultRenderDataCategories::Selection);
  if (renderDataBatchList.GetBatchCount() == 0)
  {
    return;
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  nsGALTextureHandle hDepthTexture;

  // render all selection objects to depth target only
  {
    nsUInt32 uiWidth = pColorOutput->m_Desc.m_uiWidth;
    nsUInt32 uiHeight = pColorOutput->m_Desc.m_uiHeight;
    nsGALMSAASampleCount::Enum sampleCount = pColorOutput->m_Desc.m_SampleCount;
    nsUInt32 uiSliceCount = pColorOutput->m_Desc.m_uiArraySize;

    hDepthTexture = nsGPUResourcePool::GetDefaultInstance()->GetRenderTarget(uiWidth, uiHeight, nsGALResourceFormat::D24S8, sampleCount, uiSliceCount);

    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(pDevice->GetDefaultRenderTargetView(hDepthTexture));
    renderingSetup.m_bClearDepth = true;
    renderingSetup.m_bClearStencil = true;

    auto pCommandEncoder = nsRenderContext::BeginPassAndRenderingScope(renderViewContext, std::move(renderingSetup), GetName(), renderViewContext.m_pCamera->IsStereoscopic());

    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("RENDER_PASS", "RENDER_PASS_DEPTH_ONLY");

    RenderDataWithCategory(renderViewContext, nsDefaultRenderDataCategories::Selection);
  }

  // reconstruct selection overlay from depth target
  {
    auto constants = nsRenderContext::GetConstantBufferData<nsSelectionHighlightConstants>(m_hConstantBuffer);
    constants->HighlightColor = m_HighlightColor;
    constants->OverlayOpacity = m_fOverlayOpacity;

    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(pColorOutput->m_TextureHandle));

    auto pCommandEncoder = nsRenderContext::BeginPassAndRenderingScope(renderViewContext, std::move(renderingSetup), GetName(), renderViewContext.m_pCamera->IsStereoscopic());

    renderViewContext.m_pRenderContext->BindShader(m_hShader);
    renderViewContext.m_pRenderContext->BindConstantBuffer("nsSelectionHighlightConstants", m_hConstantBuffer);
    renderViewContext.m_pRenderContext->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
    renderViewContext.m_pRenderContext->BindTexture2D("SelectionDepthTexture", pDevice->GetDefaultResourceView(hDepthTexture));
    renderViewContext.m_pRenderContext->BindTexture2D("SceneDepthTexture", pDevice->GetDefaultResourceView(pDepthInput->m_TextureHandle));

    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();

    nsGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hDepthTexture);
  }
}

nsResult nsSelectionHighlightPass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_HighlightColor;
  inout_stream << m_fOverlayOpacity;
  return NS_SUCCESS;
}

nsResult nsSelectionHighlightPass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_HighlightColor;
  inout_stream >> m_fOverlayOpacity;
  return NS_SUCCESS;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_SelectionHighlightPass);
