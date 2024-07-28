#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/Pipeline/Passes/MsaaUpscalePass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsaaUpscalePass, 2, nsRTTIDefaultAllocator<nsMsaaUpscalePass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Input", m_PinInput),
    NS_MEMBER_PROPERTY("Output", m_PinOutput),
    NS_ENUM_MEMBER_PROPERTY("MSAA_Mode", nsGALMSAASampleCount, m_MsaaMode)
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsMsaaUpscalePass::nsMsaaUpscalePass()
  : nsRenderPipelinePass("MsaaUpscalePass")

{
  {
    // Load shader.
    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/MsaaUpscale.nsShader");
    NS_ASSERT_DEV(m_hShader.IsValid(), "Could not load msaa upscale shader!");
  }
}

nsMsaaUpscalePass::~nsMsaaUpscalePass() = default;

bool nsMsaaUpscalePass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  auto pInput = inputs[m_PinInput.m_uiInputIndex];
  if (pInput != nullptr)
  {
    if (pInput->m_SampleCount != nsGALMSAASampleCount::None)
    {
      nsLog::Error("Input must not be a msaa target");
      return false;
    }

    nsGALTextureCreationDescription desc = *pInput;
    desc.m_SampleCount = m_MsaaMode;

    outputs[m_PinOutput.m_uiOutputIndex] = desc;
  }
  else
  {
    nsLog::Error("No input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void nsMsaaUpscalePass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  auto pInput = inputs[m_PinInput.m_uiInputIndex];
  auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  // Setup render target
  nsGALRenderingSetup renderingSetup;
  renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(pOutput->m_TextureHandle));

  // Bind render target and viewport
  auto pCommandEncoder = nsRenderContext::BeginPassAndRenderingScope(renderViewContext, std::move(renderingSetup), GetName(), renderViewContext.m_pCamera->IsStereoscopic());

  renderViewContext.m_pRenderContext->BindShader(m_hShader);
  renderViewContext.m_pRenderContext->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
  renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));

  renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();
}

nsResult nsMsaaUpscalePass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_MsaaMode;
  return NS_SUCCESS;
}

nsResult nsMsaaUpscalePass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_MsaaMode;
  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsMsaaUpscalePassPatch_1_2 : public nsGraphPatch
{
public:
  nsMsaaUpscalePassPatch_1_2()
    : nsGraphPatch("nsMsaaUpscalePass", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override { pNode->RenameProperty("MSAA Mode", "MSAA_Mode"); }
};

nsMsaaUpscalePassPatch_1_2 g_nsMsaaUpscalePassPatch_1_2;



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_MsaaUpscalePass);
