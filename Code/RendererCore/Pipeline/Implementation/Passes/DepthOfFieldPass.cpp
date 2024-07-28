/*
* #include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Pipeline/Passes/DepthOfFieldPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/DepthOfFieldConstants.h"

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDepthOfFieldPass, 1, nsRTTIDefaultAllocator<nsDepthOfFieldPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color", m_PinInput),
    NS_MEMBER_PROPERTY("Output", m_PinOutput),
    NS_MEMBER_PROPERTY("DepthStencil", m_PinDepth),
    NS_MEMBER_PROPERTY("Radius", m_fRadius)->AddAttributes(new nsDefaultValueAttribute(5.5f)),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsDepthOfFieldPass::nsDepthOfFieldPass()
  : nsRenderPipelinePass("DepthOfFieldPass")
  , m_fRadius(5.5f)
{
  // Loading shaders
  {
    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/DepthOfField.nsShader");
    NS_ASSERT_DEV(m_hShader.IsValid(), "Could not load DepthOfField shader!");
  }

  // Loading resources
  {
    m_hConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsDepthOfFieldConstants>();
  }
}

nsDepthOfFieldPass::~nsDepthOfFieldPass()
{
  nsRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool nsDepthOfFieldPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  // Color
  if (inputs[m_PinInput.m_uiInputIndex])
  {
    if (!inputs[m_PinInput.m_uiInputIndex]->m_bAllowShaderResourceView)
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
    nsGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }

  return true;
}

void nsDepthOfFieldPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pDepth = inputs[m_PinDepth.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pDepth == nullptr || pOutput == nullptr)
  {
    return;
  }

  nsTempHashedString sCircleOfConfusionPass = "DOF_PASS_MODE_COC";
  nsTempHashedString sBokehPass = "DOF_PASS_MODE_BOKEH";
  nsTempHashedString sTentPass = "DOF_PASS_MODE_TENT";
  nsTempHashedString sBlendPass = "DOF_PASS_MODE_UPSCALE_BLEND";

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsResourceManager::ForceLoadResourceNow(m_hShader);

  const bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  const nsUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
  const nsUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

  const nsUInt32 uiWidthHalf = uiWidth / 2;
  const nsUInt32 uiHeightHalf = uiHeight / 2;

  nsGALTextureHandle hBokehTexture1;
  nsGALTextureHandle hBokehTexture2;
  {
    nsGALTextureCreationDescription desc = pInput->m_Desc;
    desc.m_uiWidth = uiWidthHalf;
    desc.m_uiHeight = uiHeightHalf;
    desc.m_bAllowUAV = true;
    desc.m_bAllowShaderResourceView = true;

    hBokehTexture1 = pDevice->CreateTexture(desc);
    hBokehTexture2 = pDevice->CreateTexture(desc);
  }

  nsGALPass* pPass = pDevice->BeginPass(GetName());
  {
    // Circle of Confusion Pass
    {
      auto pCommandEncoder = nsRenderContext::BeginComputeScope(pPass, renderViewContext, "Circle of Confusion");
      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      nsGALUnorderedAccessViewHandle hOutput;
      {
        nsGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = hBokehTexture1;
        hOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindTexture2D("DepthTexture", pDevice->GetDefaultResourceView(pDepth->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindConstantBuffer("nsDepthOfFieldConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("DOF_PASS_MODE", sCircleOfConfusionPass);

      const nsUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
      const nsUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

      UpdateConstantBuffer();

      nsGlobalConstants& gc = renderViewContext.m_pRenderContext->WriteGlobalConstants();
      nsVec4 olViewportSize = gc.ViewportSize;

      gc.ViewportSize = nsVec4(uiWidthHalf, uiHeightHalf, 1.0f / uiWidthHalf, 1.0f / uiHeightHalf);
      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      gc.ViewportSize = olViewportSize;
    }

    // Bokeh Pass
    {
      auto pCommandEncoder = nsRenderContext::BeginComputeScope(pPass, renderViewContext, "Bokeh");
      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      nsGALUnorderedAccessViewHandle hOutput;
      {
        nsGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = hBokehTexture2;
        hOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(hBokehTexture1));
      renderViewContext.m_pRenderContext->BindConstantBuffer("nsDepthOfFieldConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("DOF_PASS_MODE", sBokehPass);

      const nsUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
      const nsUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

      UpdateConstantBuffer();

      nsGlobalConstants& gc = renderViewContext.m_pRenderContext->WriteGlobalConstants();
      nsVec4 olViewportSize = gc.ViewportSize;

      gc.ViewportSize = nsVec4(uiWidthHalf, uiHeightHalf, 1.0f / uiWidthHalf, 1.0f / uiHeightHalf);
      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      gc.ViewportSize = olViewportSize;
    }

    // Tent Pass
    {
      auto pCommandEncoder = nsRenderContext::BeginComputeScope(pPass, renderViewContext, "Tent");
      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      nsGALUnorderedAccessViewHandle hOutput;
      {
        nsGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = hBokehTexture1;
        hOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(hBokehTexture2));
      renderViewContext.m_pRenderContext->BindConstantBuffer("nsDepthOfFieldConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("DOF_PASS_MODE", sTentPass);

      const nsUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
      const nsUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

      UpdateConstantBuffer();

      nsGlobalConstants& gc = renderViewContext.m_pRenderContext->WriteGlobalConstants();
      nsVec4 olViewportSize = gc.ViewportSize;

      gc.ViewportSize = nsVec4(uiWidthHalf, uiHeightHalf, 1.0f / uiWidthHalf, 1.0f / uiHeightHalf);
      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      gc.ViewportSize = olViewportSize;
    }

    // Upscale Blend Pass
    {
      auto pCommandEncoder = nsRenderContext::BeginComputeScope(pPass, renderViewContext, "Upscale Blend");
      renderViewContext.m_pRenderContext->BindShader(m_hShader);

      nsGALUnorderedAccessViewHandle hOutput;
      {
        nsGALUnorderedAccessViewCreationDescription desc;
        desc.m_hTexture = pOutput->m_TextureHandle;
        hOutput = pDevice->CreateUnorderedAccessView(desc);
      }

      renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
      renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindTexture2D("BokehTexture", pDevice->GetDefaultResourceView(hBokehTexture1));
      renderViewContext.m_pRenderContext->BindTexture2D("DepthTexture", pDevice->GetDefaultResourceView(pDepth->m_TextureHandle));
      renderViewContext.m_pRenderContext->BindConstantBuffer("nsDepthOfFieldConstants", m_hConstantBuffer);

      renderViewContext.m_pRenderContext->SetShaderPermutationVariable("DOF_PASS_MODE", sBlendPass);

      const nsUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
      const nsUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

      UpdateConstantBuffer();

      nsGlobalConstants& gc = renderViewContext.m_pRenderContext->WriteGlobalConstants();
      nsVec4 olViewportSize = gc.ViewportSize;

      gc.ViewportSize = nsVec4(uiWidth, uiHeight, 1.0f / uiWidth, 1.0f / uiHeight);
      renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
      gc.ViewportSize = olViewportSize;
    }
  }
  pDevice->EndPass(pPass);

  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading);

  // Cleanup resources
  pDevice->DestroyTexture(hBokehTexture1);
  pDevice->DestroyTexture(hBokehTexture2);
}

void nsDepthOfFieldPass::ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  const auto pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
  {
    return;
  }

  auto pCommandEncoder = nsRenderContext::BeginPassAndComputeScope(renderViewContext, GetName());

  pCommandEncoder->CopyTexture(pOutput->m_TextureHandle, pInput->m_TextureHandle);
}

void nsDepthOfFieldPass::UpdateConstantBuffer()
{
  auto* constants = nsRenderContext::GetConstantBufferData<nsDepthOfFieldConstants>(m_hConstantBuffer);
  constants->Radius = m_fRadius;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_DepthOfFieldPass);

*/
