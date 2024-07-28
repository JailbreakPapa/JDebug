#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/AOPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererFoundation/Profiling/Profiling.h>

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/DownscaleDepthConstants.h>
#include <RendererCore/../../../Data/Base/Shaders/Pipeline/SSAOConstants.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAOPass, 1, nsRTTIDefaultAllocator<nsAOPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("DepthInput", m_PinDepthInput),
    NS_MEMBER_PROPERTY("Output", m_PinOutput),
    NS_MEMBER_PROPERTY("Radius", m_fRadius)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.01f, 10.0f)),
    NS_MEMBER_PROPERTY("MaxScreenSpaceRadius", m_fMaxScreenSpaceRadius)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.01f, 2.0f)),
    NS_MEMBER_PROPERTY("Contrast", m_fContrast)->AddAttributes(new nsDefaultValueAttribute(2.0f)),
    NS_MEMBER_PROPERTY("Intensity", m_fIntensity)->AddAttributes(new nsDefaultValueAttribute(0.7f)),
    NS_ACCESSOR_PROPERTY("FadeOutStart", GetFadeOutStart, SetFadeOutStart)->AddAttributes(new nsDefaultValueAttribute(80.0f), new nsClampValueAttribute(0.0f, nsVariant())),
    NS_ACCESSOR_PROPERTY("FadeOutEnd", GetFadeOutEnd, SetFadeOutEnd)->AddAttributes(new nsDefaultValueAttribute(100.0f), new nsClampValueAttribute(0.0f, nsVariant())),
    NS_MEMBER_PROPERTY("PositionBias", m_fPositionBias)->AddAttributes(new nsDefaultValueAttribute(5.0f), new nsClampValueAttribute(0.0f, 1000.0f)),
    NS_MEMBER_PROPERTY("MipLevelScale", m_fMipLevelScale)->AddAttributes(new nsDefaultValueAttribute(10.0f), new nsClampValueAttribute(0.0f, nsVariant())),
    NS_MEMBER_PROPERTY("DepthBlurThreshold", m_fDepthBlurThreshold)->AddAttributes(new nsDefaultValueAttribute(2.0f), new nsClampValueAttribute(0.01f, nsVariant())),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsAOPass::nsAOPass()
  : nsRenderPipelinePass("AOPass", true)

{
  m_hNoiseTexture = nsResourceManager::LoadResource<nsTexture2DResource>("Textures/SSAONoise.dds");

  m_hDownscaleShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/DownscaleDepth.nsShader");
  NS_ASSERT_DEV(m_hDownscaleShader.IsValid(), "Could not load downsample shader!");

  m_hSSAOShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/SSAO.nsShader");
  NS_ASSERT_DEV(m_hSSAOShader.IsValid(), "Could not load SSAO shader!");

  m_hBlurShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/SSAOBlur.nsShader");
  NS_ASSERT_DEV(m_hBlurShader.IsValid(), "Could not load SSAO shader!");

  m_hDownscaleConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsDownscaleDepthConstants>();
  m_hSSAOConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsSSAOConstants>();
}

nsAOPass::~nsAOPass()
{
  if (!m_hSSAOSamplerState.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroySamplerState(m_hSSAOSamplerState);
    m_hSSAOSamplerState.Invalidate();
  }

  nsRenderContext::DeleteConstantBufferStorage(m_hDownscaleConstantBuffer);
  m_hDownscaleConstantBuffer.Invalidate();

  nsRenderContext::DeleteConstantBufferStorage(m_hSSAOConstantBuffer);
  m_hSSAOConstantBuffer.Invalidate();
}

bool nsAOPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  if (auto pDepthInput = inputs[m_PinDepthInput.m_uiInputIndex])
  {
    if (!pDepthInput->m_bAllowShaderResourceView)
    {
      nsLog::Error("'{0}' input must allow shader resource view.", GetName());
      return false;
    }

    if (pDepthInput->m_SampleCount != nsGALMSAASampleCount::None)
    {
      nsLog::Error("'{0}' input must be resolved", GetName());
      return false;
    }

    nsGALTextureCreationDescription desc = *pDepthInput;
    desc.m_Format = nsGALResourceFormat::RGHalf;

    outputs[m_PinOutput.m_uiOutputIndex] = desc;
  }
  else
  {
    nsLog::Error("No input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void nsAOPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  auto pDepthInput = inputs[m_PinDepthInput.m_uiInputIndex];
  auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pDepthInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsGALPass* pGALPass = pDevice->BeginPass(GetName());
  NS_SCOPE_EXIT(pDevice->EndPass(pGALPass));

  nsUInt32 uiWidth = pDepthInput->m_Desc.m_uiWidth;
  nsUInt32 uiHeight = pDepthInput->m_Desc.m_uiHeight;

  nsUInt32 uiNumMips = 3;
  nsUInt32 uiHzbWidth = nsMath::RoundUp(uiWidth, 1u << uiNumMips);
  nsUInt32 uiHzbHeight = nsMath::RoundUp(uiHeight, 1u << uiNumMips);

  float fHzbScaleX = (float)uiWidth / uiHzbWidth;
  float fHzbScaleY = (float)uiHeight / uiHzbHeight;

  // Find temp targets
  nsGALTextureHandle hzbTexture;
  nsHybridArray<nsVec2, 8> hzbSizes;
  nsHybridArray<nsGALTextureResourceViewHandle, 8> hzbResourceViews;
  nsHybridArray<nsGALRenderTargetViewHandle, 8> hzbRenderTargetViews;

  nsGALTextureHandle tempSSAOTexture;

  {
    {
      nsGALTextureCreationDescription desc;
      desc.m_uiWidth = uiHzbWidth / 2;
      desc.m_uiHeight = uiHzbHeight / 2;
      desc.m_uiMipLevelCount = 3;
      desc.m_Type = nsGALTextureType::Texture2D;
      desc.m_Format = nsGALResourceFormat::RHalf;
      desc.m_bCreateRenderTarget = true;
      desc.m_bAllowShaderResourceView = true;
      desc.m_uiArraySize = pOutput->m_Desc.m_uiArraySize;

      hzbTexture = nsGPUResourcePool::GetDefaultInstance()->GetRenderTarget(desc);
    }

    for (nsUInt32 i = 0; i < uiNumMips; ++i)
    {
      uiHzbWidth = uiHzbWidth / 2;
      uiHzbHeight = uiHzbHeight / 2;

      hzbSizes.PushBack(nsVec2((float)uiHzbWidth, (float)uiHzbHeight));

      {
        nsGALTextureResourceViewCreationDescription desc;
        desc.m_hTexture = hzbTexture;
        desc.m_uiMostDetailedMipLevel = i;
        desc.m_uiMipLevelsToUse = 1;
        desc.m_uiArraySize = pOutput->m_Desc.m_uiArraySize;

        hzbResourceViews.PushBack(pDevice->CreateResourceView(desc));
      }

      {
        nsGALRenderTargetViewCreationDescription desc;
        desc.m_hTexture = hzbTexture;
        desc.m_uiMipLevel = i;
        desc.m_uiSliceCount = pOutput->m_Desc.m_uiArraySize;

        hzbRenderTargetViews.PushBack(pDevice->CreateRenderTargetView(desc));
      }
    }

    tempSSAOTexture = nsGPUResourcePool::GetDefaultInstance()->GetRenderTarget(uiWidth, uiHeight, nsGALResourceFormat::RGHalf, nsGALMSAASampleCount::None, pOutput->m_Desc.m_uiArraySize);
  }

  // Mip map passes
  {
    CreateSamplerState();

    for (nsUInt32 i = 0; i < uiNumMips; ++i)
    {
      nsGALTextureResourceViewHandle hInputView;
      nsVec2 pixelSize;

      if (i == 0)
      {
        hInputView = pDevice->GetDefaultResourceView(pDepthInput->m_TextureHandle);
        pixelSize = nsVec2(1.0f / uiWidth, 1.0f / uiHeight);
      }
      else
      {
        hInputView = hzbResourceViews[i - 1];
        pixelSize = nsVec2(1.0f).CompDiv(hzbSizes[i - 1]);
      }

      nsGALRenderTargetViewHandle hOutputView = hzbRenderTargetViews[i];
      nsVec2 targetSize = hzbSizes[i];

      nsGALRenderingSetup renderingSetup;
      renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, hOutputView);
      renderViewContext.m_pRenderContext->BeginRendering(pGALPass, renderingSetup, nsRectFloat(targetSize.x, targetSize.y), "SSAOMipMaps", renderViewContext.m_pCamera->IsStereoscopic());

      nsDownscaleDepthConstants* constants = nsRenderContext::GetConstantBufferData<nsDownscaleDepthConstants>(m_hDownscaleConstantBuffer);
      constants->PixelSize = pixelSize;
      constants->LinearizeDepth = (i == 0);

      renderViewContext.m_pRenderContext->BindConstantBuffer("nsDownscaleDepthConstants", m_hDownscaleConstantBuffer);
      renderViewContext.m_pRenderContext->BindShader(m_hDownscaleShader);

      renderViewContext.m_pRenderContext->BindTexture2D("DepthTexture", hInputView);
      renderViewContext.m_pRenderContext->BindSamplerState("DepthSampler", m_hSSAOSamplerState);

      renderViewContext.m_pRenderContext->BindNullMeshBuffer(nsGALPrimitiveTopology::Triangles, 1);

      renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();

      renderViewContext.m_pRenderContext->EndRendering();
    }
  }

  // Update constants
  {
    float fadeOutScale = -1.0f / nsMath::Max(0.001f, (m_fFadeOutEnd - m_fFadeOutStart));
    float fadeOutOffset = -fadeOutScale * m_fFadeOutStart + 1.0f;

    nsSSAOConstants* constants = nsRenderContext::GetConstantBufferData<nsSSAOConstants>(m_hSSAOConstantBuffer);
    constants->TexCoordsScale = nsVec2(fHzbScaleX, fHzbScaleY);
    constants->FadeOutParams = nsVec2(fadeOutScale, fadeOutOffset);
    constants->WorldRadius = m_fRadius;
    constants->MaxScreenSpaceRadius = m_fMaxScreenSpaceRadius;
    constants->Contrast = m_fContrast;
    constants->Intensity = m_fIntensity;
    constants->PositionBias = m_fPositionBias / 1000.0f;
    constants->MipLevelScale = m_fMipLevelScale;
    constants->DepthBlurScale = 1.0f / m_fDepthBlurThreshold;
  }

  // SSAO pass
  {
    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(tempSSAOTexture));
    auto pCommandEncoder = renderViewContext.m_pRenderContext->BeginRenderingScope(pGALPass, renderViewContext, renderingSetup, "SSAO", renderViewContext.m_pCamera->IsStereoscopic());

    renderViewContext.m_pRenderContext->BindConstantBuffer("nsSSAOConstants", m_hSSAOConstantBuffer);
    renderViewContext.m_pRenderContext->BindShader(m_hSSAOShader);

    renderViewContext.m_pRenderContext->BindTexture2D("DepthTexture", pDevice->GetDefaultResourceView(pDepthInput->m_TextureHandle));
    renderViewContext.m_pRenderContext->BindTexture2D("LowResDepthTexture", pDevice->GetDefaultResourceView(hzbTexture));
    renderViewContext.m_pRenderContext->BindSamplerState("DepthSampler", m_hSSAOSamplerState);

    renderViewContext.m_pRenderContext->BindTexture2D("NoiseTexture", m_hNoiseTexture, nsResourceAcquireMode::BlockTillLoaded);

    renderViewContext.m_pRenderContext->BindNullMeshBuffer(nsGALPrimitiveTopology::Triangles, 1);

    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();
  }

  // Blur pass
  {
    nsGALRenderingSetup renderingSetup;
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(pOutput->m_TextureHandle));
    auto pCommandEncoder = renderViewContext.m_pRenderContext->BeginRenderingScope(pGALPass, renderViewContext, renderingSetup, "Blur", renderViewContext.m_pCamera->IsStereoscopic());

    renderViewContext.m_pRenderContext->BindConstantBuffer("nsSSAOConstants", m_hSSAOConstantBuffer);
    renderViewContext.m_pRenderContext->BindShader(m_hBlurShader);

    renderViewContext.m_pRenderContext->BindTexture2D("SSAOTexture", pDevice->GetDefaultResourceView(tempSSAOTexture));

    renderViewContext.m_pRenderContext->BindNullMeshBuffer(nsGALPrimitiveTopology::Triangles, 1);

    renderViewContext.m_pRenderContext->DrawMeshBuffer().IgnoreResult();
  }

  // Return temp targets
  if (!hzbTexture.IsInvalidated())
  {
    nsGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(hzbTexture);
  }

  if (!tempSSAOTexture.IsInvalidated())
  {
    nsGPUResourcePool::GetDefaultInstance()->ReturnRenderTarget(tempSSAOTexture);
  }
}

void nsAOPass::ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pOutput == nullptr)
  {
    return;
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  nsGALRenderingSetup renderingSetup;
  renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(pOutput->m_TextureHandle));
  renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;
  renderingSetup.m_ClearColor = nsColor::White;

  auto pCommandEncoder = nsRenderContext::BeginPassAndRenderingScope(renderViewContext, renderingSetup, GetName());
}

nsResult nsAOPass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_fRadius;
  inout_stream << m_fMaxScreenSpaceRadius;
  inout_stream << m_fContrast;
  inout_stream << m_fIntensity;
  inout_stream << m_fFadeOutStart;
  inout_stream << m_fFadeOutEnd;
  inout_stream << m_fPositionBias;
  inout_stream << m_fMipLevelScale;
  inout_stream << m_fDepthBlurThreshold;
  return NS_SUCCESS;
}

nsResult nsAOPass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_fRadius;
  inout_stream >> m_fMaxScreenSpaceRadius;
  inout_stream >> m_fContrast;
  inout_stream >> m_fIntensity;
  inout_stream >> m_fFadeOutStart;
  inout_stream >> m_fFadeOutEnd;
  inout_stream >> m_fPositionBias;
  inout_stream >> m_fMipLevelScale;
  inout_stream >> m_fDepthBlurThreshold;
  return NS_SUCCESS;
}

void nsAOPass::SetFadeOutStart(float fStart)
{
  m_fFadeOutStart = nsMath::Clamp(fStart, 0.0f, m_fFadeOutEnd);
}

float nsAOPass::GetFadeOutStart() const
{
  return m_fFadeOutStart;
}

void nsAOPass::SetFadeOutEnd(float fEnd)
{
  if (m_fFadeOutEnd == fEnd)
    return;

  m_fFadeOutEnd = nsMath::Max(fEnd, m_fFadeOutStart);

  if (!m_hSSAOSamplerState.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroySamplerState(m_hSSAOSamplerState);
    m_hSSAOSamplerState.Invalidate();
  }
}

float nsAOPass::GetFadeOutEnd() const
{
  return m_fFadeOutEnd;
}

void nsAOPass::CreateSamplerState()
{
  if (m_hSSAOSamplerState.IsInvalidated())
  {
    nsGALSamplerStateCreationDescription desc;
    desc.m_MinFilter = nsGALTextureFilterMode::Point;
    desc.m_MagFilter = nsGALTextureFilterMode::Point;
    desc.m_MipFilter = nsGALTextureFilterMode::Point;
    desc.m_AddressU = nsImageAddressMode::ClampBorder;
    desc.m_AddressV = nsImageAddressMode::ClampBorder;
    desc.m_AddressW = nsImageAddressMode::ClampBorder;
    desc.m_BorderColor = nsColor::White * m_fFadeOutEnd;

    m_hSSAOSamplerState = nsGALDevice::GetDefaultDevice()->CreateSamplerState(desc);
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_AOPass);
