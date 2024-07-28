#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Lights/SimplifiedDataExtractor.h>
#include <RendererCore/Lights/SimplifiedDataProvider.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Profiling/Profiling.h>

#include <RendererCore/../../../Data/Base/Shaders/Common/LightDataSimplified.h>
NS_DEFINE_AS_POD_TYPE(nsSimplifiedDataConstants);

nsSimplifiedDataGPU::nsSimplifiedDataGPU()
{
  m_hConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsSimplifiedDataConstants>();
}

nsSimplifiedDataGPU::~nsSimplifiedDataGPU()
{
  nsRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

void nsSimplifiedDataGPU::BindResources(nsRenderContext* pRenderContext)
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  auto hReflectionSpecularTextureView = pDevice->GetDefaultResourceView(nsReflectionPool::GetReflectionSpecularTexture(m_uiSkyIrradianceIndex, m_cameraUsageHint));
  auto hSkyIrradianceTextureView = pDevice->GetDefaultResourceView(nsReflectionPool::GetSkyIrradianceTexture());

  pRenderContext->BindTextureCube("ReflectionSpecularTexture", hReflectionSpecularTextureView);
  pRenderContext->BindTexture2D("SkyIrradianceTexture", hSkyIrradianceTextureView);

  pRenderContext->BindConstantBuffer("nsSimplifiedDataConstants", m_hConstantBuffer);
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSimplifiedDataProvider, 1, nsRTTIDefaultAllocator<nsSimplifiedDataProvider>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSimplifiedDataProvider::nsSimplifiedDataProvider() = default;

nsSimplifiedDataProvider::~nsSimplifiedDataProvider() = default;

void* nsSimplifiedDataProvider::UpdateData(const nsRenderViewContext& renderViewContext, const nsExtractedRenderData& extractedData)
{
  nsGALCommandEncoder* pGALCommandEncoder = renderViewContext.m_pRenderContext->GetRenderCommandEncoder();

  NS_PROFILE_AND_MARKER(pGALCommandEncoder, "Update Clustered Data");

  if (auto pData = extractedData.GetFrameData<nsSimplifiedDataCPU>())
  {
    m_Data.m_uiSkyIrradianceIndex = pData->m_uiSkyIrradianceIndex;
    m_Data.m_cameraUsageHint = pData->m_cameraUsageHint;

    // Update Constants
    const nsRectFloat& viewport = renderViewContext.m_pViewData->m_ViewPortRect;

    nsSimplifiedDataConstants* pConstants =
      renderViewContext.m_pRenderContext->GetConstantBufferData<nsSimplifiedDataConstants>(m_Data.m_hConstantBuffer);

    pConstants->SkyIrradianceIndex = pData->m_uiSkyIrradianceIndex;
  }

  return &m_Data;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_SimplifiedDataProvider);
