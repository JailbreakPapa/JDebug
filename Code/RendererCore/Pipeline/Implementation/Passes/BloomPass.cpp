#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/BloomPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/BloomConstants.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsBloomPass, 1, nsRTTIDefaultAllocator<nsBloomPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Input", m_PinInput),
    NS_MEMBER_PROPERTY("Output", m_PinOutput),
    NS_MEMBER_PROPERTY("Radius", m_fRadius)->AddAttributes(new nsDefaultValueAttribute(0.2f), new nsClampValueAttribute(0.01f, 1.0f)),
    NS_MEMBER_PROPERTY("Threshold", m_fThreshold)->AddAttributes(new nsDefaultValueAttribute(1.0f)),
    NS_MEMBER_PROPERTY("Intensity", m_fIntensity)->AddAttributes(new nsDefaultValueAttribute(0.3f)),
    NS_MEMBER_PROPERTY("InnerTintColor", m_InnerTintColor),
    NS_MEMBER_PROPERTY("MidTintColor", m_MidTintColor),
    NS_MEMBER_PROPERTY("OuterTintColor", m_OuterTintColor),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsBloomPass::nsBloomPass()
  : nsRenderPipelinePass("BloomPass", true)
{
  {
    // Load shader.
    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/Bloom.nsShader");
    NS_ASSERT_DEV(m_hShader.IsValid(), "Could not load bloom shader!");
  }

  {
    m_hConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsBloomConstants>();
  }
}

nsBloomPass::~nsBloomPass()
{
  nsRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool nsBloomPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinInput.m_uiInputIndex])
  {
    if (!inputs[m_PinInput.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      nsLog::Error("'{0}' input must allow shader resource view.", GetName());
      return false;
    }

    // Output is half-res
    nsGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
    desc.m_uiWidth = desc.m_uiWidth / 2;
    desc.m_uiHeight = desc.m_uiHeight / 2;
    desc.m_Format = nsGALResourceFormat::RG11B10Float;

    outputs[m_PinOutput.m_uiOutputIndex] = desc;
  }
  else
  {
    nsLog::Error("No input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void nsBloomPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  auto pColorInput = inputs[m_PinInput.m_uiInputIndex];
  auto pColorOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pColorInput == nullptr || pColorOutput == nullptr)
  {
    return;
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsGALPass* pGALPass = pDevice->BeginPass(GetName());
  NS_SCOPE_EXIT(pDevice->EndPass(pGALPass));

  nsUInt32 uiWidth = pColorInput->m_Desc.m_uiWidth;
  nsUInt32 uiHeight = pColorInput->m_Desc.m_uiHeight;
  bool bFastDownscale = nsMath::IsEven(uiWidth) && nsMath::IsEven(uiHeight);

  const float fMaxRes = (float)nsMath::Max(uiWidth, uiHeight);
  const float fRadius = nsMath::Clamp(m_fRadius, 0.01f, 1.0f);
  const float fDownscaledSize = 4.0f / fRadius;
  const float fNumBlurPasses = nsMath::Log2(fMaxRes / fDownscaledSize);
  const nsUInt32 uiNumBlurPasses = (nsUInt32)nsMath::Ceil(fNumBlurPasses);

  // Find temp targets
  nsHybridArray<nsVec2, 8> targetSizes;
  nsHybridArray<nsGALTextureHandle, 8> tempDownscaleTextures;
  nsHybridArray<nsGALTextureHandle, 8> tempUpscaleTextures;

  for (nsUInt32 i = 0; i < uiNumBlurPasses; ++i)
  {
    uiWidth = nsMath::Max(uiWidth / 2, 1u);
    uiHeight = nsMath::Max(uiHeight / 2, 1u);
    targetSizes.PushBack(nsVec2((float)uiWidth, (float)uiHeight));
    auto uiSliceCount = pColorOutput->m_Desc.m_uiArraySize;

    tempDownscaleTextures.PushBack(nsGPUResourcePool::GetDefaultInstance()->GetRenderTarget(uiWidth, uiHeight, nsGALResourceFormat::RG11B10Float, nsGALMSAASampleCount::None, uiSliceCount));

    // biggest upscale target is the output and lowest is not needed
    if (i > 0 && i < uiNumBlurPasses - 1)
    {
      tempUpscaleTextures.PushBack(nsGPUResourcePool::GetDefaultInstance()->GetRenderTarget(uiWidth, uiHeight, nsGALResourceFormat::RG11B10Float, nsGALMSAASampleCount::None, uiSliceCount));
    }
    else
    {
      tempUpscaleTextures.PushBack(nsGALTextureHandle());
    }
  }

  renderViewContext.m_pRenderContext->BindConstantBuffer("nsBloomConstants", m_hConstantBuffer);
  renderViewContext.m_pRenderContext->BindShader(m_hShader);

  renderViewContext.m_pRenderContext->BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, nsGALPrimitiveTopology::Triangles, 1);

  // Downscale passes
  {
    nsTempHashedString sInitialDownscale = "BLOOM_PASS_MODE_INITIAL_DOWNSCALE";
    nsTempHashedString sInitialDownscaleFast = "BLOOM_PASS_MODE_INITIAL_DOWNSCALE_FAST";
    nsTempHashedString sDownscale = "BLOOM_PASS_MODE_DOWNSCALE";
    nsTempHashedString sDownscaleFast = "BLOOM_PASS_MODE_DOWNSCALE_FAST";

    for (nsUInt32 i = 0; i < uiNumBlurPasses; ++i)
    {
      nsGALTextureHandle hInput;
      if (i == 0)
      {
        hInput = pColorInput->m_TextureHandle;
        renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLOOM_PASS_MODE", bFastDownscale ? sInitialDownscaleFast : sInitialDownscale);
      }
      else
      {
        hInput = tempDownscaleTextures[i - 1];
        renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLOOM_PASS_MODE", bFastDownscale ? sDownscaleFast : sDownscale);
      }

      nsGALTextureHandle hOutput = tempDownscaleTextures[i];
      nsVec2 targetSize = targetSizes[i];

      nsGALRenderingSetup renderingSetup;
      renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(hOutput));
      renderViewContext.m_pRenderContext->BeginRendering(pGALPass, renderingSetup, nsRectFloat(targetSize.x, targetSize.y), "Downscale", renderViewContext.m_pCamera->IsStereoscopic());

      nsColor tintColor = (i == uiNumBlurPasses - 1) ? nsColor(m_OuterTintColor) : nsColor::White;
      UpdateConstantBuffer(nsVec2(1.0f).CompDiv(targetSize), tintColor);

      renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", pDevice->GetDefaultResourceView(hInput));
      renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();

      renderViewContext.m_pRenderContext->EndRendering();

      bFastDownscale = nsMath::IsEven((nsInt32)targetSize.x) && nsMath::IsEven((nsInt32)targetSize.y);
    }
  }

  // Upscale passes
  {
    const float fBlurRadius = 2.0f * fNumBlurPasses / uiNumBlurPasses;
    const float fMidPass = (uiNumBlurPasses - 1.0f) / 2.0f;

    renderViewContext.m_pRenderContext->SetShaderPermutationVariable("BLOOM_PASS_MODE", "BLOOM_PASS_MODE_UPSCALE");

    for (nsUInt32 i = uiNumBlurPasses - 1; i-- > 0;)
    {
      nsGALTextureHandle hNextInput = tempDownscaleTextures[i];
      nsGALTextureHandle hInput;
      if (i == uiNumBlurPasses - 2)
      {
        hInput = tempDownscaleTextures[i + 1];
      }
      else
      {
        hInput = tempUpscaleTextures[i + 1];
      }

      nsGALTextureHandle hOutput;
      if (i == 0)
      {
        hOutput = pColorOutput->m_TextureHandle;
      }
      else
      {
        hOutput = tempUpscaleTextures[i];
      }

      nsVec2 targetSize = targetSizes[i];

      nsGALRenderingSetup renderingSetup;
      renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(hOutput));
      renderViewContext.m_pRenderContext->BeginRendering(pGALPass, renderingSetup, nsRectFloat(targetSize.x, targetSize.y), "Upscale", renderViewContext.m_pCamera->IsStereoscopic());

      nsColor tintColor;
      float fPass = (float)i;
      if (fPass < fMidPass)
      {
        tintColor = nsMath::Lerp<nsColor>(m_InnerTintColor, m_MidTintColor, fPass / fMidPass);
      }
      else
      {
        tintColor = nsMath::Lerp<nsColor>(m_MidTintColor, m_OuterTintColor, (fPass - fMidPass) / fMidPass);
      }

      UpdateConstantBuffer(nsVec2(fBlurRadius).CompDiv(targetSize), tintColor);

      renderViewContext.m_pRenderContext->BindTexture2D("NextColorTexture", pDevice->GetDefaultResourceView(hNextInput));
      renderViewContext.m_pRenderContext->BindTexture2D("ColorTexture", pDevice->GetDefaultResourceView(hInput));
      renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();

      renderViewContext.m_pRenderContext->EndRendering();
    }
  }

  // Return temp targets
  for (auto hTexture : tempDownscaleTextures)
  {
    if (!hTexture.IsInvalidated())
    {
      nsGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hTexture);
    }
  }

  for (auto hTexture : tempUpscaleTextures)
  {
    if (!hTexture.IsInvalidated())
    {
      nsGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hTexture);
    }
  }
}

void nsBloomPass::ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  auto pColorOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pColorOutput == nullptr)
  {
    return;
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  nsGALRenderingSetup renderingSetup;
  renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(pColorOutput->m_TextureHandle));
  renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;
  renderingSetup.m_ClearColor = nsColor::Black;

  auto pCommandEncoder = nsRenderContext::BeginPassAndRenderingScope(renderViewContext, renderingSetup, "Clear");
}

nsResult nsBloomPass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_fRadius;
  inout_stream << m_fThreshold;
  inout_stream << m_fIntensity;
  inout_stream << m_InnerTintColor;
  inout_stream << m_MidTintColor;
  inout_stream << m_OuterTintColor;
  return NS_SUCCESS;
}

nsResult nsBloomPass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_fRadius;
  inout_stream >> m_fThreshold;
  inout_stream >> m_fIntensity;
  inout_stream >> m_InnerTintColor;
  inout_stream >> m_MidTintColor;
  inout_stream >> m_OuterTintColor;
  return NS_SUCCESS;
}

void nsBloomPass::UpdateConstantBuffer(nsVec2 pixelSize, const nsColor& tintColor)
{
  nsBloomConstants* constants = nsRenderContext::GetConstantBufferData<nsBloomConstants>(m_hConstantBuffer);
  constants->PixelSize = pixelSize;
  constants->BloomThreshold = m_fThreshold;
  constants->BloomIntensity = m_fIntensity;

  constants->TintColor = tintColor;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_BloomPass);
