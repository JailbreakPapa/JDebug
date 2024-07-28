#pragma once

#include <Foundation/Types/UniquePtr.h>
#include <RendererCore/Pipeline/Extractor.h>

struct nsPerLightData;
struct nsPerDecalData;
struct nsPerReflectionProbeData;
struct nsPerClusterData;

class nsClusteredDataCPU : public nsRenderData
{
  NS_ADD_DYNAMIC_REFLECTION(nsClusteredDataCPU, nsRenderData);

public:
  nsClusteredDataCPU();
  ~nsClusteredDataCPU();

  enum
  {
    MAX_LIGHT_DATA = 1024,
    MAX_DECAL_DATA = 1024,
    MAX_REFLECTION_PROBE_DATA = 1024,
    MAX_ITEMS_PER_CLUSTER = 256
  };

  nsArrayPtr<nsPerLightData> m_LightData;
  nsArrayPtr<nsPerDecalData> m_DecalData;
  nsArrayPtr<nsPerReflectionProbeData> m_ReflectionProbeData;
  nsArrayPtr<nsPerClusterData> m_ClusterData;
  nsArrayPtr<nsUInt32> m_ClusterItemList;

  nsUInt32 m_uiSkyIrradianceIndex = 0;
  nsEnum<nsCameraUsageHint> m_cameraUsageHint = nsCameraUsageHint::Default;

  float m_fFogHeight = 0.0f;
  float m_fFogHeightFalloff = 0.0f;
  float m_fFogDensityAtCameraPos = 0.0f;
  float m_fFogDensity = 0.0f;
  float m_fFogInvSkyDistance = 0.0f;
  nsColor m_FogColor = nsColor::Black;
};

class NS_RENDERERCORE_DLL nsClusteredDataExtractor : public nsExtractor
{
  NS_ADD_DYNAMIC_REFLECTION(nsClusteredDataExtractor, nsExtractor);

public:
  nsClusteredDataExtractor(const char* szName = "ClusteredDataExtractor");
  ~nsClusteredDataExtractor();

  virtual void PostSortAndBatch(
    const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData) override;
  virtual nsResult Serialize(nsStreamWriter& inout_stream) const override;
  virtual nsResult Deserialize(nsStreamReader& inout_stream) override;

private:
  void FillItemListAndClusterData(nsClusteredDataCPU* pData);

  template <nsUInt32 MaxData>
  struct TempCluster
  {
    NS_DECLARE_POD_TYPE();

    nsUInt32 m_BitMask[MaxData / 32];
  };

  nsDynamicArray<nsPerLightData, nsAlignedAllocatorWrapper> m_TempLightData;
  nsDynamicArray<nsPerDecalData, nsAlignedAllocatorWrapper> m_TempDecalData;
  nsDynamicArray<nsPerReflectionProbeData, nsAlignedAllocatorWrapper> m_TempReflectionProbeData;
  nsDynamicArray<TempCluster<nsClusteredDataCPU::MAX_LIGHT_DATA>> m_TempLightsClusters;
  nsDynamicArray<TempCluster<nsClusteredDataCPU::MAX_DECAL_DATA>> m_TempDecalsClusters;
  nsDynamicArray<TempCluster<nsClusteredDataCPU::MAX_REFLECTION_PROBE_DATA>> m_TempReflectionProbeClusters;
  nsDynamicArray<nsUInt32> m_TempClusterItemList;

  nsDynamicArray<nsSimdBSphere, nsAlignedAllocatorWrapper> m_ClusterBoundingSpheres;
};
