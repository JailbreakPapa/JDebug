#include <RendererCore/RendererCorePCH.h>

#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Pipeline/Passes/ReflectionFilterPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

#include <RendererCore/../../../Data/Base/Shaders/Pipeline/ReflectionFilteredSpecularConstants.h>
#include <RendererCore/../../../Data/Base/Shaders/Pipeline/ReflectionIrradianceConstants.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsReflectionFilterPass, 1, nsRTTIDefaultAllocator<nsReflectionFilterPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("FilteredSpecular", m_PinFilteredSpecular),
    NS_MEMBER_PROPERTY("AvgLuminance", m_PinAvgLuminance),
    NS_MEMBER_PROPERTY("IrradianceData", m_PinIrradianceData),
    NS_MEMBER_PROPERTY("Intensity", m_fIntensity)->AddAttributes(new nsDefaultValueAttribute(1.0f)),
    NS_MEMBER_PROPERTY("Saturation", m_fSaturation)->AddAttributes(new nsDefaultValueAttribute(1.0f)),
    NS_MEMBER_PROPERTY("SpecularOutputIndex", m_uiSpecularOutputIndex),
    NS_MEMBER_PROPERTY("IrradianceOutputIndex", m_uiIrradianceOutputIndex),
    NS_ACCESSOR_PROPERTY("InputCubemap", GetInputCubemap, SetInputCubemap)
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsReflectionFilterPass::nsReflectionFilterPass()
  : nsRenderPipelinePass("ReflectionFilterPass")

{
  {
    m_hFilteredSpecularConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsReflectionFilteredSpecularConstants>();
    m_hFilteredSpecularShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/ReflectionFilteredSpecular.nsShader");
    NS_ASSERT_DEV(m_hFilteredSpecularShader.IsValid(), "Could not load ReflectionFilteredSpecular shader!");

    m_hIrradianceConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsReflectionIrradianceConstants>();
    m_hIrradianceShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/ReflectionIrradiance.nsShader");
    NS_ASSERT_DEV(m_hIrradianceShader.IsValid(), "Could not load ReflectionIrradiance shader!");
  }
}

nsReflectionFilterPass::~nsReflectionFilterPass()
{
  nsRenderContext::DeleteConstantBufferStorage(m_hIrradianceConstantBuffer);
  m_hIrradianceConstantBuffer.Invalidate();
}

bool nsReflectionFilterPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  {
    nsGALTextureCreationDescription desc;
    desc.m_uiWidth = nsReflectionPool::GetReflectionCubeMapSize();
    desc.m_uiHeight = desc.m_uiWidth;
    desc.m_Format = nsGALResourceFormat::RGBAHalf;
    desc.m_Type = nsGALTextureType::TextureCube;
    desc.m_bAllowUAV = true;
    desc.m_uiMipLevelCount = nsMath::Log2i(desc.m_uiWidth) - 1;
    outputs[m_PinFilteredSpecular.m_uiOutputIndex] = desc;
  }

  return true;
}

void nsReflectionFilterPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  auto pInputCubemap = pDevice->GetTexture(m_hInputCubemap);
  if (pInputCubemap == nullptr)
  {
    return;
  }

  // We cannot allow the filter to work on fallback resources as the step will not be repeated for static cube maps. Thus, we force loading the shaders and disable async shader loading in this scope.
  nsResourceManager::ForceLoadResourceNow(m_hFilteredSpecularShader);
  nsResourceManager::ForceLoadResourceNow(m_hIrradianceShader);
  bool bAllowAsyncShaderLoading = renderViewContext.m_pRenderContext->GetAllowAsyncShaderLoading();
  renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(false);

  nsGALPass* pGALPass = pDevice->BeginPass(GetName());
  NS_SCOPE_EXIT(
    pDevice->EndPass(pGALPass);
    renderViewContext.m_pRenderContext->SetAllowAsyncShaderLoading(bAllowAsyncShaderLoading));

  if (pInputCubemap->GetDescription().m_bAllowDynamicMipGeneration)
  {
    auto pCommandEncoder = nsRenderContext::BeginRenderingScope(pGALPass, renderViewContext, nsGALRenderingSetup(), "MipMaps");
    pCommandEncoder->GenerateMipMaps(pDevice->GetDefaultResourceView(m_hInputCubemap));
  }

  {
    auto pFilteredSpecularOutput = outputs[m_PinFilteredSpecular.m_uiOutputIndex];
    if (pFilteredSpecularOutput != nullptr && !pFilteredSpecularOutput->m_TextureHandle.IsInvalidated())
    {
      nsUInt32 uiNumMipMaps = pFilteredSpecularOutput->m_Desc.m_uiMipLevelCount;

      nsUInt32 uiWidth = pFilteredSpecularOutput->m_Desc.m_uiWidth;
      nsUInt32 uiHeight = pFilteredSpecularOutput->m_Desc.m_uiHeight;

      auto pCommandEncoder = nsRenderContext::BeginComputeScope(pGALPass, renderViewContext, "ReflectionFilter");
      renderViewContext.m_pRenderContext->BindTextureCube("InputCubemap", pDevice->GetDefaultResourceView(m_hInputCubemap));
      renderViewContext.m_pRenderContext->BindConstantBuffer("nsReflectionFilteredSpecularConstants", m_hFilteredSpecularConstantBuffer);
      renderViewContext.m_pRenderContext->BindShader(m_hFilteredSpecularShader);

      for (nsUInt32 uiMipMapIndex = 0; uiMipMapIndex < uiNumMipMaps; ++uiMipMapIndex)
      {
        nsGALTextureUnorderedAccessViewHandle hFilterOutput;
        {
          nsGALTextureUnorderedAccessViewCreationDescription desc;
          desc.m_hTexture = pFilteredSpecularOutput->m_TextureHandle;
          desc.m_uiMipLevelToUse = uiMipMapIndex;
          desc.m_uiFirstArraySlice = m_uiSpecularOutputIndex * 6;
          desc.m_uiArraySize = 6;
          hFilterOutput = pDevice->CreateUnorderedAccessView(desc);
        }
        renderViewContext.m_pRenderContext->BindUAV("ReflectionOutput", hFilterOutput);
        UpdateFilteredSpecularConstantBuffer(uiMipMapIndex, uiNumMipMaps);

        constexpr nsUInt32 uiThreadsX = 8;
        constexpr nsUInt32 uiThreadsY = 8;
        const nsUInt32 uiDispatchX = (uiWidth + uiThreadsX - 1) / uiThreadsX;
        const nsUInt32 uiDispatchY = (uiHeight + uiThreadsY - 1) / uiThreadsY;

        renderViewContext.m_pRenderContext->Dispatch(uiDispatchX, uiDispatchY, 6).IgnoreResult();

        uiWidth >>= 1;
        uiHeight >>= 1;
      }
    }
  }

  auto pIrradianceOutput = outputs[m_PinIrradianceData.m_uiOutputIndex];
  if (pIrradianceOutput != nullptr && !pIrradianceOutput->m_TextureHandle.IsInvalidated())
  {
    auto pCommandEncoder = nsRenderContext::BeginComputeScope(pGALPass, renderViewContext, "Irradiance");

    nsGALTextureUnorderedAccessViewHandle hIrradianceOutput;
    {
      nsGALTextureUnorderedAccessViewCreationDescription desc;
      desc.m_hTexture = pIrradianceOutput->m_TextureHandle;

      hIrradianceOutput = pDevice->CreateUnorderedAccessView(desc);
    }
    renderViewContext.m_pRenderContext->BindUAV("IrradianceOutput", hIrradianceOutput);

    renderViewContext.m_pRenderContext->BindTextureCube("InputCubemap", pDevice->GetDefaultResourceView(m_hInputCubemap));

    UpdateIrradianceConstantBuffer();

    renderViewContext.m_pRenderContext->BindConstantBuffer("nsReflectionIrradianceConstants", m_hIrradianceConstantBuffer);
    renderViewContext.m_pRenderContext->BindShader(m_hIrradianceShader);

    renderViewContext.m_pRenderContext->Dispatch(1).IgnoreResult();
  }
}

nsResult nsReflectionFilterPass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_fIntensity;
  inout_stream << m_fSaturation;
  inout_stream << m_uiSpecularOutputIndex;
  inout_stream << m_uiIrradianceOutputIndex;
  // inout_stream << m_hInputCubemap; Runtime only property
  return NS_SUCCESS;
}

nsResult nsReflectionFilterPass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_fIntensity;
  inout_stream >> m_fSaturation;
  inout_stream >> m_uiSpecularOutputIndex;
  inout_stream >> m_uiIrradianceOutputIndex;
  return NS_SUCCESS;
}

nsUInt32 nsReflectionFilterPass::GetInputCubemap() const
{
  return m_hInputCubemap.GetInternalID().m_Data;
}

void nsReflectionFilterPass::SetInputCubemap(nsUInt32 uiCubemapHandle)
{
  m_hInputCubemap = nsGALTextureHandle(nsGAL::ns18_14Id(uiCubemapHandle));
}

void nsReflectionFilterPass::UpdateFilteredSpecularConstantBuffer(nsUInt32 uiMipMapIndex, nsUInt32 uiNumMipMaps)
{
  auto constants = nsRenderContext::GetConstantBufferData<nsReflectionFilteredSpecularConstants>(m_hFilteredSpecularConstantBuffer);
  constants->MipLevel = uiMipMapIndex;
  constants->Intensity = m_fIntensity;
  constants->Saturation = m_fSaturation;
}

void nsReflectionFilterPass::UpdateIrradianceConstantBuffer()
{
  auto constants = nsRenderContext::GetConstantBufferData<nsReflectionIrradianceConstants>(m_hIrradianceConstantBuffer);
  constants->LodLevel = 6; // TODO: calculate from cubemap size and number of samples
  constants->Intensity = m_fIntensity;
  constants->Saturation = m_fSaturation;
  constants->OutputIndex = m_uiIrradianceOutputIndex;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_ReflectionFilterPass);
