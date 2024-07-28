#pragma once

#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/FrameDataProvider.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>

struct NS_RENDERERCORE_DLL nsSimplifiedDataGPU
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsSimplifiedDataGPU);

public:
  nsSimplifiedDataGPU();
  ~nsSimplifiedDataGPU();

  nsUInt32 m_uiSkyIrradianceIndex = 0;
  nsEnum<nsCameraUsageHint> m_cameraUsageHint = nsCameraUsageHint::Default;
  nsConstantBufferStorageHandle m_hConstantBuffer;

  void BindResources(nsRenderContext* pRenderContext);
};

class NS_RENDERERCORE_DLL nsSimplifiedDataProvider : public nsFrameDataProvider<nsSimplifiedDataGPU>
{
  NS_ADD_DYNAMIC_REFLECTION(nsSimplifiedDataProvider, nsFrameDataProviderBase);

public:
  nsSimplifiedDataProvider();
  ~nsSimplifiedDataProvider();

private:
  virtual void* UpdateData(const nsRenderViewContext& renderViewContext, const nsExtractedRenderData& extractedData) override;

  nsSimplifiedDataGPU m_Data;
};
