#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/SSRPass.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Resources/RenderTargetSetup.h>
#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/SSRConstants.h>
#include <RendererCore/../../../Data/Base/Shaders/Common/GlobalConstants.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSSRPass, 1, nsRTTIDefaultAllocator<nsSSRPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_PinColor),
    NS_MEMBER_PROPERTY("DepthInput", m_PinDepthStencil),
    NS_MEMBER_PROPERTY("Output", m_PinOutput),
    NS_MEMBER_PROPERTY("MaxRayDistance", maxDistance)->AddAttributes(new nsDefaultValueAttribute(8.0f)),
    NS_MEMBER_PROPERTY("Resolution", resolution)->AddAttributes(new nsDefaultValueAttribute(0.3f)),
    NS_MEMBER_PROPERTY("Steps", steps)->AddAttributes(new nsDefaultValueAttribute(5)),
    NS_MEMBER_PROPERTY("Thickness", thickness)->AddAttributes(new nsDefaultValueAttribute(0.5f)),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSSRPass::nsSSRPass(const char* szName)
  : nsRenderPipelinePass(szName, true)
{
  // Load shader.
  m_hSSRShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/SSR.nsShader");
  NS_ASSERT_DEV(m_hSSRShader.IsValid(), "Could not load Screen Space Reflection shader!");

  m_hSSRConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsSSRConstants>();
}

nsSSRPass::~nsSSRPass()
{
  if (!m_hSSRSamplerState.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroySamplerState(m_hSSRSamplerState);
    m_hSSRSamplerState.Invalidate();
  }
  if (!m_hSSRColorSamplerState.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroySamplerState(m_hSSRColorSamplerState);
    m_hSSRColorSamplerState.Invalidate();
  }
  nsRenderContext::DeleteConstantBufferStorage(m_hSSRConstantBuffer);
  m_hSSRConstantBuffer.Invalidate();
}

bool nsSSRPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  if (auto pDepthInput = inputs[m_PinDepthStencil.m_uiInputIndex])
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
  }
  // Color
  if (inputs[m_PinColor.m_uiInputIndex])
  {
    if (!inputs[m_PinColor.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
     nsLog::Error("'{0}' color input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    nsLog::Error("No color input connected to '{0}'!", GetName());
    return false;
  }
  {
    nsGALTextureCreationDescription desc = *inputs[m_PinColor.m_uiInputIndex];
    desc.m_Format = nsGALResourceFormat::Default;
    desc.m_bAllowUAV = true;
    desc.m_bCreateRenderTarget = true;
    outputs[m_PinOutput.m_uiOutputIndex] = desc;
  }

  return true;
}

void nsSSRPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  const auto pColorInput = inputs[m_PinColor.m_uiInputIndex];
  const auto pDepthInput = inputs[m_PinDepthStencil.m_uiInputIndex];
  const auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pColorInput == nullptr || pDepthInput == nullptr || pOutput == nullptr)
    return;

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

  nsGALTextureHandle TSSRTexture;
  {
    {
      nsGALTextureCreationDescription desc;
      desc.m_uiWidth = uiHzbWidth / 2;
      desc.m_uiHeight = uiHzbHeight / 2;
      desc.m_uiMipLevelCount = 3;
      desc.m_Type = nsGALTextureType::Texture2D;
      desc.m_Format = nsGALResourceFormat::Default;
      desc.m_bCreateRenderTarget = true;
      desc.m_bAllowShaderResourceView = true;
      desc.m_uiArraySize = pOutput->m_Desc.m_uiArraySize;
      // TODO: https://github.com/mateeeeeee/Adria-DX12/blob/master/Adria/Rendering/SSRPass.cpp

      hzbTexture = nsGPUResourcePool::GetDefaultInstance()->GetRenderTarget(desc);
    }
  }
}

void nsSSRPass::ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  const auto pColorInput = inputs[m_PinColor.m_uiInputIndex];
  const auto pDepthInput = inputs[m_PinDepthStencil.m_uiInputIndex];
  const auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];
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

nsResult nsSSRPass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << maxDistance;
  inout_stream << resolution;
  inout_stream << resolution;
  inout_stream << steps;
  inout_stream << thickness;
  return NS_SUCCESS;
}

nsResult nsSSRPass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> maxDistance;
  inout_stream >> resolution;
  inout_stream >> resolution;
  inout_stream >> steps;
  inout_stream >> thickness;
  return NS_SUCCESS;
}

void nsSSRPass::CreateSamplerState()
{
  if (m_hSSRSamplerState.IsInvalidated())
  {
    nsGALSamplerStateCreationDescription desc;
    desc.m_MinFilter = nsGALTextureFilterMode::Point;
    desc.m_MagFilter = nsGALTextureFilterMode::Point;
    desc.m_MipFilter = nsGALTextureFilterMode::Point;
    desc.m_AddressU = nsImageAddressMode::ClampBorder;
    desc.m_AddressV = nsImageAddressMode::ClampBorder;
    desc.m_AddressW = nsImageAddressMode::ClampBorder;
    desc.m_BorderColor = nsColor::White;

    m_hSSRSamplerState = nsGALDevice::GetDefaultDevice()->CreateSamplerState(desc);
  }
  if (m_hSSRColorSamplerState.IsInvalidated())
  {
    nsGALSamplerStateCreationDescription desc;
    desc.m_MinFilter = nsGALTextureFilterMode::Point;
    desc.m_MagFilter = nsGALTextureFilterMode::Point;
    desc.m_MipFilter = nsGALTextureFilterMode::Point;
    desc.m_AddressU = nsImageAddressMode::ClampBorder;
    desc.m_AddressV = nsImageAddressMode::ClampBorder;
    desc.m_AddressW = nsImageAddressMode::ClampBorder;
    desc.m_BorderColor = nsColor::White;

    m_hSSRColorSamplerState = nsGALDevice::GetDefaultDevice()->CreateSamplerState(desc);
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_SSRPass);
