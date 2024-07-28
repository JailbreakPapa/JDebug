#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/TAAPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/CopyConstants.h"
#include "../../../../../../Data/Base/Shaders/Pipeline/TAAConstants.h"

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTAAPass, 1, nsRTTIDefaultAllocator<nsTAAPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Input", m_PinInputColor),
    NS_MEMBER_PROPERTY("Velocity", m_PinInputVelocity),
    NS_MEMBER_PROPERTY("DepthStencil", m_PinInputDepth),
    NS_MEMBER_PROPERTY("Output", m_PinOutput),
    NS_MEMBER_PROPERTY("UpscaleEnabled", m_bUpsample)->AddAttributes(new nsDefaultValueAttribute(false)),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on


nsTAAPass::nsTAAPass()
  : nsRenderPipelinePass("TAAPass", true)
  , m_bUpsample(false)
{
  // Load shader
  {
    m_hTAAShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/TAA.nsShader");
    NS_ASSERT_DEV(m_hTAAShader.IsValid(), "Could not load TAA Pass shader!");

    m_hCopyShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/Copy_CS.nsShader");
    NS_ASSERT_DEV(m_hCopyShader.IsValid(), "Could not load the Copy texture shader required for TAA!");
  }

  // Load resources
  {
    m_hTAAConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsTAAConstants>();
    m_hCopyConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsCopyConstants>();
  }
}

nsTAAPass::~nsTAAPass()
{
  nsRenderContext::DeleteConstantBufferStorage(m_hCopyConstantBuffer);
  m_hCopyConstantBuffer.Invalidate();

  nsRenderContext::DeleteConstantBufferStorage(m_hTAAConstantBuffer);
  m_hTAAConstantBuffer.Invalidate();

  if (!m_hHistory.IsInvalidated())
  {
    nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

    pDevice->DestroyTexture(m_hHistory);
    m_hHistory.Invalidate();
  }

  if (!m_hPreviousVelocity.IsInvalidated())
  {
    nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

    pDevice->DestroyTexture(m_hPreviousVelocity);
    m_hPreviousVelocity.Invalidate();
  }
}

bool nsTAAPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinInputColor.m_uiInputIndex])
  {
    if (!inputs[m_PinInputColor.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      nsLog::Error("'{0}' Color input must allow shader resource view.", GetName());
      return false;
    }

    nsGALTextureCreationDescription desc = *inputs[m_PinInputColor.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    desc.m_bCreateRenderTarget = true;

    if (m_bUpsample)
    {
      desc.m_uiWidth = nsMath::Max(static_cast<nsUInt32>(view.GetViewport().width), desc.m_uiWidth);
      desc.m_uiHeight = nsMath::Max(static_cast<nsUInt32>(view.GetViewport().height), desc.m_uiHeight);
    }

    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }
  else
  {
    nsLog::Error("No Color input connected to '{0}'!", GetName());
    return false;
  }

  // Velocity
  if (inputs[m_PinInputVelocity.m_uiInputIndex])
  {
    if (!inputs[m_PinInputVelocity.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      nsLog::Error("'{0}' Velocity input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    nsLog::Error("No Velocity input connected to '{0}'!", GetName());
    return false;
  }

  // Depth - Stencil
  if (inputs[m_PinInputDepth.m_uiInputIndex])
  {
    if (!inputs[m_PinInputDepth.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      nsLog::Error("'{0}' Depth input must allow shader resource view.", GetName());
      return false;
    }
  }
  else
  {
    nsLog::Error("No depth/stencil input connected to pass '{0}'.", GetName());
    return false;
  }

  return true;
}

void nsTAAPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
 
}

void nsTAAPass::ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInputColor.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  const nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  const nsGALTexture* pDest = pDevice->GetTexture(pOutput->m_TextureHandle);

  if (const nsGALTexture* pSource = pDevice->GetTexture(pInput->m_TextureHandle); pDest->GetDescription().m_Format != pSource->GetDescription().m_Format)
  {
    // TODO: use a shader when the format doesn't match exactly

    nsLog::Error("Copying textures of different formats is not implemented");
  }
  else
  {
    auto pCommandEncoder = nsRenderContext::BeginPassAndComputeScope(renderViewContext, GetName());

    pCommandEncoder->CopyTexture(pOutput->m_TextureHandle, pInput->m_TextureHandle);
  }
}

void nsTAAPass::UpdateTAAConstantBuffer() const
{
  auto* constants = nsRenderContext::GetConstantBufferData<nsTAAConstants>(m_hTAAConstantBuffer);
  constants->UpsampleEnabled = m_bUpsample;
}

void nsTAAPass::UpdateCopyConstantBuffer(nsVec2I32 offset, nsVec2U32 size) const
{
  auto* constants = nsRenderContext::GetConstantBufferData<nsCopyConstants>(m_hCopyConstantBuffer);
  constants->Offset = offset;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_TAAPass);
