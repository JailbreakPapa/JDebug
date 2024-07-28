/*#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Pipeline/Passes/FilmGrainPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>

#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#include "../../../../../../Data/Base/Shaders/Pipeline/FilmGrainConstants.h"

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsFilmGrainPass, 1, nsRTTIDefaultAllocator<nsFilmGrainPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Input", m_PinInput),
    NS_MEMBER_PROPERTY("Output", m_PinOutput),
    NS_MEMBER_PROPERTY("Intensity", m_fIntensity)->AddAttributes(new nsDefaultValueAttribute(0.002f), new nsClampValueAttribute(0.0f, 1.0f)),
    NS_MEMBER_PROPERTY("Speed", m_fSpeed)->AddAttributes(new nsDefaultValueAttribute(3.0f)),
    NS_MEMBER_PROPERTY("Mean", m_fMean)->AddAttributes(new nsDefaultValueAttribute(0.0f)),
    NS_MEMBER_PROPERTY("Variance", m_fVariance)->AddAttributes(new nsDefaultValueAttribute(0.5f)),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsFilmGrainPass::nsFilmGrainPass()
  : nsRenderPipelinePass("FilmGrainPass", true)
  , m_fIntensity(0.002f)
  , m_fSpeed(3.0f)
  , m_fMean(0.0f)
  , m_fVariance(0.5f)
{
  // Load shader.
  {
    m_hShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/FilmGrain.nsShader");
    NS_ASSERT_DEV(m_hShader.IsValid(), "Could not load FilmGrain Pass shader!");
  }

  // Load resources.
  {
    m_hConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsFilmGrainConstants>();
  }
}

nsFilmGrainPass::~nsFilmGrainPass()
{
  nsRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
  m_hConstantBuffer.Invalidate();
}

bool nsFilmGrainPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  // Input
  if (inputs[m_PinInput.m_uiInputIndex])
  {
    if (!inputs[m_PinInput.m_uiInputIndex]->m_bAllowShaderResourceView)
    {
      nsLog::Error("'{0}' Color input must allow shader resource view.", GetName());
      return false;
    }

    nsGALTextureCreationDescription desc = *inputs[m_PinInput.m_uiInputIndex];
    desc.m_bAllowUAV = true;
    desc.m_bCreateRenderTarget = true;
    outputs[m_PinOutput.m_uiOutputIndex] = std::move(desc);
  }
  else
  {
    nsLog::Error("No Color input connected to '{0}'!", GetName());
    return false;
  }

  return true;
}

void nsFilmGrainPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
    return;

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  nsResourceManager::ForceLoadResourceNow(m_hShader);

  const bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  nsGALPass* pPass = pDevice->BeginPass(GetName());
  {
    auto pCommandEncoder = nsRenderContext::BeginComputeScope(pPass, renderViewContext);

    renderViewContext.m_pRenderContext->BindShader(m_hShader);

    nsGALUnorderedAccessViewHandle hOutput;
    {
      nsGALUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pOutput->m_TextureHandle;
      desc.m_uiMipLevelToUse = 0;
      hOutput = pDevice->CreateUnorderedAccessView(desc);
    }

    renderViewContext.m_pRenderContext->BindUAV("Output", hOutput);
    renderViewContext.m_pRenderContext->BindTexture2D("InputTexture", pDevice->GetDefaultResourceView(pInput->m_TextureHandle));
    renderViewContext.m_pRenderContext->BindConstantBuffer("nsFilmGrainConstants", m_hConstantBuffer);

    const nsUInt32 uiWidth = pOutput->m_Desc.m_uiWidth;
    const nsUInt32 uiHeight = pOutput->m_Desc.m_uiHeight;

    const nsUInt32 uiDispatchX = (uiWidth + THREAD_GROUP_COUNT_X - 1) / THREAD_GROUP_COUNT_X;
    const nsUInt32 uiDispatchY = (uiHeight + THREAD_GROUP_COUNT_Y - 1) / THREAD_GROUP_COUNT_Y;

    UpdateConstantBuffer();

    renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 1).IgnoreResult();
  }
  pDevice->EndPass(pPass);

  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading);
}

void nsFilmGrainPass::ExecuteInactive(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  const auto* const pInput = inputs[m_PinInput.m_uiInputIndex];
  const auto* const pOutput = outputs[m_PinOutput.m_uiOutputIndex];

  if (pInput == nullptr || pOutput == nullptr)
    return;

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

void nsFilmGrainPass::UpdateConstantBuffer() const
{
  auto* constants = nsRenderContext::GetConstantBufferData<nsFilmGrainConstants>(m_hConstantBuffer);
  constants->Intensity = m_fIntensity;
  constants->Speed = m_fSpeed;
  constants->Mean = m_fMean;
  constants->Variance = m_fVariance;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_FilmGrainPass);
*/
