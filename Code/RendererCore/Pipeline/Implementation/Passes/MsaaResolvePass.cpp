#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/Passes/MsaaResolvePass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsaaResolvePass, 1, nsRTTIDefaultAllocator<nsMsaaResolvePass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Input", m_PinInput),
    NS_MEMBER_PROPERTY("Output", m_PinOutput)
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsMsaaResolvePass::nsMsaaResolvePass()
  : nsRenderPipelinePass("MsaaResolvePass", true)

{
  {
    // Load shader.
    m_hDepthResolveShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/MsaaDepthResolve.nsShader");
    NS_ASSERT_DEV(m_hDepthResolveShader.IsValid(), "Could not load depth resolve shader!");
  }
}

nsMsaaResolvePass::~nsMsaaResolvePass() = default;

bool nsMsaaResolvePass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  auto pInput = inputs[m_PinInput.m_uiInputIndex];
  if (pInput != nullptr)
  {
    if (pInput->m_SampleCount == nsGALMSAASampleCount::None)
    {
      nsLog::Error("Input is not a valid msaa target");
      return false;
    }

    m_bIsDepth = nsGALResourceFormat::IsDepthFormat(pInput->m_Format);
    m_MsaaSampleCount = pInput->m_SampleCount;

    nsGALTextureCreationDescription desc = *pInput;
    desc.m_SampleCount = nsGALMSAASampleCount::None;

    outputs[m_PinOutput.m_uiOutputIndex] = desc;
  }
  else
  {
    nsLog::Error("No input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void nsMsaaResolvePass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  auto pInput = inputs[m_PinInput.m_uiInputIndex];
  auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  if (m_bIsDepth)
  {
    // Setup render target
    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(pDevice->GetDefaultRenderTargetView(pOutput->m_TextureHandle));

    // Bind render target and viewport
    auto pCommandEncoder = nsRenderContext::BeginPassAndRenderingScope(renderViewContext, std::move(renderingSetup), GetName(), renderViewContext.m_pCamera->IsStereoscopic());

    auto& globals = renderViewContext.m_pRenderContext->WriteGlobalConstants();
    globals.NumMsaaSamples = m_MsaaSampleCount;

    renderViewContext.m_pRenderContext->BindShader(m_hDepthResolveShader);
    renderViewContext.m_pRenderContext->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
    renderViewContext.m_pRenderContext->BindTexture2D("DepthTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));

    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();
  }
  else
  {
    auto pCommandEncoder = nsRenderContext::BeginPassAndRenderingScope(renderViewContext, nsGALRenderingSetup(), GetName(), renderViewContext.m_pCamera->IsStereoscopic());

    nsGALTextureSubresource subresource;
    subresource.m_uiMipLevel = 0;
    subresource.m_uiArraySlice = 0;

    pCommandEncoder->ResolveTexture(pOutput->m_TextureHandle, subresource, pInput->m_TextureHandle, subresource);

    if (renderViewContext.m_pCamera->IsStereoscopic())
    {
      subresource.m_uiArraySlice = 1;
      pCommandEncoder->ResolveTexture(pOutput->m_TextureHandle, subresource, pInput->m_TextureHandle, subresource);
    }
  }
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_MsaaResolvePass);
