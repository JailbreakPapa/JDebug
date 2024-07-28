#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/SeparatedBilateralBlur.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <Core/Graphics/Geometry.h>
#include <RendererCore/../../../Data/Base/Shaders/Pipeline/BilateralBlurConstants.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSeparatedBilateralBlurPass, 2, nsRTTIDefaultAllocator<nsSeparatedBilateralBlurPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("BlurSource", m_PinBlurSourceInput),
    NS_MEMBER_PROPERTY("Depth", m_PinDepthInput),
    NS_MEMBER_PROPERTY("Output", m_PinOutput),
    NS_ACCESSOR_PROPERTY("BlurRadius", GetRadius, SetRadius)->AddAttributes(new nsDefaultValueAttribute(7)),
      // Should we really expose that? This gives the user control over the error compared to a perfect gaussian.
      // In theory we could also compute this for a given error from the blur radius. See http://dev.theomader.com/gaussian-kernel-calculator/ for visualization.
    NS_ACCESSOR_PROPERTY("GaussianSigma", GetGaussianSigma, SetGaussianSigma)->AddAttributes(new nsDefaultValueAttribute(4.0f)),
    NS_ACCESSOR_PROPERTY("Sharpness", GetSharpness, SetSharpness)->AddAttributes(new nsDefaultValueAttribute(120.0f)),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSeparatedBilateralBlurPass::nsSeparatedBilateralBlurPass()
  : nsRenderPipelinePass("SeparatedBilateral")

{
  {
    // Load shader.
    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/SeparatedBilateralBlur.nsShader");
    NS_ASSERT_DEV(m_hShader.IsValid(), "Could not load blur shader!");
  }

  {
    m_hBilateralBlurCB = nsRenderContext::CreateConstantBufferStorage<nsBilateralBlurConstants>();
  }
}

nsSeparatedBilateralBlurPass::~nsSeparatedBilateralBlurPass()
{
  nsRenderContext::DeleteConstantBufferStorage(m_hBilateralBlurCB);
  m_hBilateralBlurCB.Invalidate();
}

bool nsSeparatedBilateralBlurPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  NS_ASSERT_DEBUG(inputs.GetCount() == 2, "Unexpected number of inputs for nsSeparatedBilateralBlurPass.");

  // Color
  if (!inputs[m_PinBlurSourceInput.m_uiInputIndex])
  {
    nsLog::Error("No blur target connected to bilateral blur pass!");
    return false;
  }
  if (!inputs[m_PinBlurSourceInput.m_uiInputIndex]->m_bAllowShaderResourceView)
  {
    nsLog::Error("All bilateral blur pass inputs must allow shader resoure view.");
    return false;
  }

  // Depth
  if (!inputs[m_PinDepthInput.m_uiInputIndex])
  {
    nsLog::Error("No depth connected to bilateral blur pass!");
    return false;
  }
  if (!inputs[m_PinDepthInput.m_uiInputIndex]->m_bAllowShaderResourceView)
  {
    nsLog::Error("All bilateral blur pass inputs must allow shader resoure view.");
    return false;
  }
  if (inputs[m_PinBlurSourceInput.m_uiInputIndex]->m_uiWidth != inputs[m_PinDepthInput.m_uiInputIndex]->m_uiWidth || inputs[m_PinBlurSourceInput.m_uiInputIndex]->m_uiHeight != inputs[m_PinDepthInput.m_uiInputIndex]->m_uiHeight)
  {
    nsLog::Error("Blur target and depth buffer for bilateral blur pass need to have the same dimensions.");
    return false;
  }


  // Output format maches input format.
  outputs[m_PinOutput.m_uiOutputIndex] = *inputs[m_PinBlurSourceInput.m_uiInputIndex];

  return true;
}

void nsSeparatedBilateralBlurPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  if (outputs[m_PinOutput.m_uiOutputIndex])
  {
    nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
    nsGALPass* pGALPass = pDevice->BeginPass(GetName());
    NS_SCOPE_EXIT(pDevice->EndPass(pGALPass));

    // Setup input view and sampler
    nsGALTextureResourceViewCreationDescription rvcd;
    rvcd.m_hTexture = inputs[m_PinBlurSourceInput.m_uiInputIndex]->m_TextureHandle;
    nsGALTextureResourceViewHandle hBlurSourceInputView = nsGALDevice::GetDefaultDevice()->CreateResourceView(rvcd);
    rvcd.m_hTexture = inputs[m_PinDepthInput.m_uiInputIndex]->m_TextureHandle;
    nsGALTextureResourceViewHandle hDepthInputView = nsGALDevice::GetDefaultDevice()->CreateResourceView(rvcd);

    // Get temp texture for horizontal target / vertical source.
    nsGALTextureCreationDescription tempTextureDesc = outputs[m_PinBlurSourceInput.m_uiInputIndex]->m_Desc;
    tempTextureDesc.m_bAllowShaderResourceView = true;
    tempTextureDesc.m_bCreateRenderTarget = true;
    nsGALTextureHandle tempTexture = nsGPUResourcePool::GetDefaultInstance()->GetRenderTarget(tempTextureDesc);
    rvcd.m_hTexture = tempTexture;
    nsGALTextureResourceViewHandle hTempTextureRView = nsGALDevice::GetDefaultDevice()->CreateResourceView(rvcd);

    nsGALRenderingSetup renderingSetup;

    // Bind shader and inputs
    renderViewContext.m_pRenderContext->BindShader(m_hShader);
    renderViewContext.m_pRenderContext->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);
    renderViewContext.m_pRenderContext->BindTexture2D("DepthBuffer", hDepthInputView);
    renderViewContext.m_pRenderContext->BindConstantBuffer("nsBilateralBlurConstants", m_hBilateralBlurCB);

    // Horizontal
    {
      renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(tempTexture));
      auto pCommandEncoder = nsRenderContext::BeginRenderingScope(pGALPass, renderViewContext, renderingSetup, "", renderViewContext.m_pCamera->IsStereoscopic());

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLUR_DIRECTION", "BLUR_DIRECTION_HORIZONTAL");
      renderViewContext.m_pRenderContext->BindTexture2D("BlurSource", hBlurSourceInputView);
      renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();
    }

    // Vertical
    {
      renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(outputs[m_PinOutput.m_uiOutputIndex]->m_TextureHandle));
      auto pCommandEncoder = nsRenderContext::BeginRenderingScope(pGALPass, renderViewContext, renderingSetup, "", renderViewContext.m_pCamera->IsStereoscopic());

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLUR_DIRECTION", "BLUR_DIRECTION_VERTICAL");
      renderViewContext.m_pRenderContext->BindTexture2D("BlurSource", hTempTextureRView);
      renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();
    }

    // Give back temp texture.
    nsGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(tempTexture);
  }
}

nsResult nsSeparatedBilateralBlurPass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_uiRadius;
  inout_stream << m_fGaussianSigma;
  inout_stream << m_fSharpness;
  return NS_SUCCESS;
}

nsResult nsSeparatedBilateralBlurPass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_uiRadius;
  inout_stream >> m_fGaussianSigma;
  inout_stream >> m_fSharpness;
  return NS_SUCCESS;
}

void nsSeparatedBilateralBlurPass::SetRadius(nsUInt32 uiRadius)
{
  m_uiRadius = uiRadius;

  nsBilateralBlurConstants* cb = nsRenderContext::GetConstantBufferData<nsBilateralBlurConstants>(m_hBilateralBlurCB);
  cb->BlurRadius = m_uiRadius;
}

nsUInt32 nsSeparatedBilateralBlurPass::GetRadius() const
{
  return m_uiRadius;
}

void nsSeparatedBilateralBlurPass::SetGaussianSigma(const float fSigma)
{
  m_fGaussianSigma = fSigma;

  nsBilateralBlurConstants* cb = nsRenderContext::GetConstantBufferData<nsBilateralBlurConstants>(m_hBilateralBlurCB);
  cb->GaussianFalloff = 1.0f / (2.0f * m_fGaussianSigma * m_fGaussianSigma);
}

float nsSeparatedBilateralBlurPass::GetGaussianSigma() const
{
  return m_fGaussianSigma;
}

void nsSeparatedBilateralBlurPass::SetSharpness(const float fSharpness)
{
  m_fSharpness = fSharpness;

  nsBilateralBlurConstants* cb = nsRenderContext::GetConstantBufferData<nsBilateralBlurConstants>(m_hBilateralBlurCB);
  cb->Sharpness = m_fSharpness;
}

float nsSeparatedBilateralBlurPass::GetSharpness() const
{
  return m_fSharpness;
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsSeparatedBilateralBlurPassPatch_1_2 : public nsGraphPatch
{
public:
  nsSeparatedBilateralBlurPassPatch_1_2()
    : nsGraphPatch("nsSeparatedBilateralBlurPass", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    pNode->RenameProperty("Blur Radius", "BlurRadius");
    pNode->RenameProperty("Gaussian Standard Deviation", "GaussianSigma");
    pNode->RenameProperty("Bilateral Sharpness", "Sharpness");
  }
};

nsSeparatedBilateralBlurPassPatch_1_2 g_nsSeparatedBilateralBlurPassPatch_1_2;



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_SeparatedBilateralBlur);
