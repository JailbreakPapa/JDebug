#pragma once

#include <RendererCore/Pipeline/Extractor.h>

class nsSimplifiedDataCPU : public nsRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsSimplifiedDataCPU, nsRenderData);

public:
  nsSimplifiedDataCPU();
  ~nsSimplifiedDataCPU();

  nsUInt32 m_uiSkyIrradianceIndex = 0;
  nsEnum<nsCameraUsageHint> m_cameraUsageHint = nsCameraUsageHint::Default;
};

class NS_RENDERERCORE_DLL nsSimplifiedDataExtractor : public nsExtractor
{
  NS_ADD_DYNAMIC_REFLECTION(nsSimplifiedDataExtractor, nsExtractor);

public:
  nsSimplifiedDataExtractor(const char* szName = "SimplifiedDataExtractor");
  ~nsSimplifiedDataExtractor();

  virtual void PostSortAndBatch(
    const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;
};
