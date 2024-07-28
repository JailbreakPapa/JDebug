#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/FrameDataProvider.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>

struct NS_RENDERERCORE_DLL nsClusteredDataGPU
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsClusteredDataGPU);

public:
  nsClusteredDataGPU();
  ~nsClusteredDataGPU();

  nsUInt32 m_uiSkyIrradianceIndex = 0;
  nsEnum<nsCameraUsageHint> m_cameraUsageHint = nsCameraUsageHint::Default;

  nsGALBufferHandle m_hLightDataBuffer;
  nsGALBufferHandle m_hDecalDataBuffer;
  nsGALBufferHandle m_hReflectionProbeDataBuffer;
  nsGALBufferHandle m_hClusterDataBuffer;
  nsGALBufferHandle m_hClusterItemBuffer;

  nsConstantBufferStorageHandle m_hConstantBuffer;

  nsGALSamplerStateHandle m_hShadowSampler;

  nsDecalAtlasResourceHandle m_hDecalAtlas;
  nsGALSamplerStateHandle m_hDecalAtlasSampler;

  void BindResources(nsRenderContext* pRenderContext);
};

class NS_RENDERERCORE_DLL nsClusteredDataProvider : public nsFrameDataProvider<nsClusteredDataGPU>
{
  NS_ADD_DYNAMIC_REFLECTION(nsClusteredDataProvider, nsFrameDataProviderBase);

public:
  nsClusteredDataProvider();
  ~nsClusteredDataProvider();

private:
  virtual void* UpdateData(const nsRenderViewContext& renderViewContext, const nsExtractedRenderData& extractedData) override;

  nsClusteredDataGPU m_Data;
};
