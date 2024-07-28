#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Decals/DecalAtlasResource.h>
#include <RendererCore/Lights/ClusteredDataExtractor.h>
#include <RendererCore/Lights/ClusteredDataProvider.h>
#include <RendererCore/Lights/Implementation/ClusteredDataUtils.h>
#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Lights/Implementation/ShadowPool.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/Textures/TextureUtils.h>
#include <RendererFoundation/Profiling/Profiling.h>

nsClusteredDataGPU::nsClusteredDataGPU()
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  {
    nsGALBufferCreationDescription desc;

    {
      desc.m_uiStructSize = sizeof(nsPerLightData);
      desc.m_uiTotalSize = desc.m_uiStructSize * nsClusteredDataCPU::MAX_LIGHT_DATA;
      desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
      desc.m_ResourceAccess.m_bImmutable = false;

      m_hLightDataBuffer = pDevice->CreateBuffer(desc);
    }

    {
      desc.m_uiStructSize = sizeof(nsPerDecalData);
      desc.m_uiTotalSize = desc.m_uiStructSize * nsClusteredDataCPU::MAX_DECAL_DATA;
      desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
      desc.m_ResourceAccess.m_bImmutable = false;

      m_hDecalDataBuffer = pDevice->CreateBuffer(desc);
    }

    {
      desc.m_uiStructSize = sizeof(nsPerReflectionProbeData);
      desc.m_uiTotalSize = desc.m_uiStructSize * nsClusteredDataCPU::MAX_REFLECTION_PROBE_DATA;
      desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ShaderResource;
      desc.m_ResourceAccess.m_bImmutable = false;

      m_hReflectionProbeDataBuffer = pDevice->CreateBuffer(desc);
    }

    {
      desc.m_uiStructSize = sizeof(nsPerClusterData);
      desc.m_uiTotalSize = desc.m_uiStructSize * NUM_CLUSTERS;

      m_hClusterDataBuffer = pDevice->CreateBuffer(desc);
    }

    {
      desc.m_uiStructSize = sizeof(nsUInt32);
      desc.m_uiTotalSize = desc.m_uiStructSize * nsClusteredDataCPU::MAX_ITEMS_PER_CLUSTER * NUM_CLUSTERS;

      m_hClusterItemBuffer = pDevice->CreateBuffer(desc);
    }
  }

  m_hConstantBuffer = nsRenderContext::CreateConstantBufferStorage<nsClusteredDataConstants>();

  {
    nsGALSamplerStateCreationDescription desc;
    desc.m_AddressU = nsImageAddressMode::Clamp;
    desc.m_AddressV = nsImageAddressMode::Clamp;
    desc.m_AddressW = nsImageAddressMode::Clamp;
    desc.m_SampleCompareFunc = nsGALCompareFunc::Less;

    m_hShadowSampler = pDevice->CreateSamplerState(desc);
  }

  m_hDecalAtlas = nsDecalAtlasResource::GetDecalAtlasResource();

  {
    nsGALSamplerStateCreationDescription desc;
    desc.m_AddressU = nsImageAddressMode::Clamp;
    desc.m_AddressV = nsImageAddressMode::Clamp;
    desc.m_AddressW = nsImageAddressMode::Clamp;

    nsTextureUtils::ConfigureSampler(nsTextureFilterSetting::DefaultQuality, desc);
    desc.m_uiMaxAnisotropy = nsMath::Min(desc.m_uiMaxAnisotropy, 4u);

    m_hDecalAtlasSampler = pDevice->CreateSamplerState(desc);
  }
}

nsClusteredDataGPU::~nsClusteredDataGPU()
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  pDevice->DestroyBuffer(m_hLightDataBuffer);
  pDevice->DestroyBuffer(m_hDecalDataBuffer);
  pDevice->DestroyBuffer(m_hReflectionProbeDataBuffer);
  pDevice->DestroyBuffer(m_hClusterDataBuffer);
  pDevice->DestroyBuffer(m_hClusterItemBuffer);
  pDevice->DestroySamplerState(m_hShadowSampler);
  pDevice->DestroySamplerState(m_hDecalAtlasSampler);

  nsRenderContext::DeleteConstantBufferStorage(m_hConstantBuffer);
}

void nsClusteredDataGPU::BindResources(nsRenderContext* pRenderContext)
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  auto hShadowDataBufferView = pDevice->GetDefaultResourceView(nsShadowPool::GetShadowDataBuffer());
  auto hShadowAtlasTextureView = pDevice->GetDefaultResourceView(nsShadowPool::GetShadowAtlasTexture());

  auto hReflectionSpecularTextureView = pDevice->GetDefaultResourceView(nsReflectionPool::GetReflectionSpecularTexture(m_uiSkyIrradianceIndex, m_cameraUsageHint));
  auto hSkyIrradianceTextureView = pDevice->GetDefaultResourceView(nsReflectionPool::GetSkyIrradianceTexture());

  pRenderContext->BindBuffer("perLightDataBuffer", pDevice->GetDefaultResourceView(m_hLightDataBuffer));
  pRenderContext->BindBuffer("perDecalDataBuffer", pDevice->GetDefaultResourceView(m_hDecalDataBuffer));
  pRenderContext->BindBuffer("perPerReflectionProbeDataBuffer", pDevice->GetDefaultResourceView(m_hReflectionProbeDataBuffer));
  pRenderContext->BindBuffer("perClusterDataBuffer", pDevice->GetDefaultResourceView(m_hClusterDataBuffer));
  pRenderContext->BindBuffer("clusterItemBuffer", pDevice->GetDefaultResourceView(m_hClusterItemBuffer));

  pRenderContext->BindBuffer("shadowDataBuffer", hShadowDataBufferView);
  pRenderContext->BindTexture2D("ShadowAtlasTexture", hShadowAtlasTextureView);
  pRenderContext->BindSamplerState("ShadowSampler", m_hShadowSampler);

  nsResourceLock<nsDecalAtlasResource> pDecalAtlas(m_hDecalAtlas, nsResourceAcquireMode::AllowLoadingFallback);
  pRenderContext->BindTexture2D("DecalAtlasBaseColorTexture", pDecalAtlas->GetBaseColorTexture());
  pRenderContext->BindTexture2D("DecalAtlasNormalTexture", pDecalAtlas->GetNormalTexture());
  pRenderContext->BindTexture2D("DecalAtlasORMTexture", pDecalAtlas->GetORMTexture());
  pRenderContext->BindSamplerState("DecalAtlasSampler", m_hDecalAtlasSampler);

  pRenderContext->BindTextureCube("ReflectionSpecularTexture", hReflectionSpecularTextureView);
  pRenderContext->BindTexture2D("SkyIrradianceTexture", hSkyIrradianceTextureView);

  pRenderContext->BindConstantBuffer("nsClusteredDataConstants", m_hConstantBuffer);
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsClusteredDataProvider, 1, nsRTTIDefaultAllocator<nsClusteredDataProvider>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsClusteredDataProvider::nsClusteredDataProvider() = default;

nsClusteredDataProvider::~nsClusteredDataProvider() = default;

void* nsClusteredDataProvider::UpdateData(const nsRenderViewContext& renderViewContext, const nsExtractedRenderData& extractedData)
{
  nsGALCommandEncoder* pGALCommandEncoder = renderViewContext.m_pRenderContext->GetRenderCommandEncoder();

  NS_PROFILE_AND_MARKER(pGALCommandEncoder, "Update Clustered Data");

  if (auto pData = extractedData.GetFrameData<nsClusteredDataCPU>())
  {
    m_Data.m_uiSkyIrradianceIndex = pData->m_uiSkyIrradianceIndex;
    m_Data.m_cameraUsageHint = pData->m_cameraUsageHint;

    // Update buffer
    if (!pData->m_ClusterItemList.IsEmpty())
    {
      if (!pData->m_LightData.IsEmpty())
      {
        pGALCommandEncoder->UpdateBuffer(m_Data.m_hLightDataBuffer, 0, pData->m_LightData.ToByteArray());
      }

      if (!pData->m_DecalData.IsEmpty())
      {
        pGALCommandEncoder->UpdateBuffer(m_Data.m_hDecalDataBuffer, 0, pData->m_DecalData.ToByteArray());
      }

      if (!pData->m_ReflectionProbeData.IsEmpty())
      {
        pGALCommandEncoder->UpdateBuffer(m_Data.m_hReflectionProbeDataBuffer, 0, pData->m_ReflectionProbeData.ToByteArray());
      }

      pGALCommandEncoder->UpdateBuffer(m_Data.m_hClusterItemBuffer, 0, pData->m_ClusterItemList.ToByteArray());
    }

    pGALCommandEncoder->UpdateBuffer(m_Data.m_hClusterDataBuffer, 0, pData->m_ClusterData.ToByteArray());

    // Update Constants
    const nsRectFloat& viewport = renderViewContext.m_pViewData->m_ViewPortRect;

    nsClusteredDataConstants* pConstants =
      renderViewContext.m_pRenderContext->GetConstantBufferData<nsClusteredDataConstants>(m_Data.m_hConstantBuffer);
    pConstants->DepthSliceScale = s_fDepthSliceScale;
    pConstants->DepthSliceBias = s_fDepthSliceBias;
    pConstants->InvTileSize = nsVec2(NUM_CLUSTERS_X / viewport.width, NUM_CLUSTERS_Y / viewport.height);
    pConstants->NumLights = pData->m_LightData.GetCount();
    pConstants->NumDecals = pData->m_DecalData.GetCount();

    pConstants->SkyIrradianceIndex = pData->m_uiSkyIrradianceIndex;

    pConstants->FogHeight = pData->m_fFogHeight;
    pConstants->FogHeightFalloff = pData->m_fFogHeightFalloff;
    pConstants->FogDensityAtCameraPos = pData->m_fFogDensityAtCameraPos;
    pConstants->FogDensity = pData->m_fFogDensity;
    pConstants->FogColor = pData->m_FogColor;
    pConstants->FogInvSkyDistance = pData->m_fFogInvSkyDistance;
  }

  return &m_Data;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_ClusteredDataProvider);
