#pragma once

#include <Core/World/World.h>
#include <Foundation/Math/Color16f.h>
#include <Foundation/Types/Bitflags.h>
#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Lights/Implementation/ReflectionProbeData.h>
#include <RendererCore/Lights/Implementation/ReflectionProbeMapping.h>
#include <RendererCore/Lights/Implementation/ReflectionProbeUpdater.h>
#include <RendererCore/Pipeline/View.h>

class nsSkyLightComponent;
class nsSphereReflectionProbeComponent;
class nsBoxReflectionProbeComponent;

static constexpr nsUInt32 s_uiReflectionCubeMapSize = 128;
static constexpr nsUInt32 s_uiNumReflectionProbeCubeMaps = 32;
static constexpr float s_fDebugSphereRadius = 0.3f;

inline nsUInt32 GetMipLevels()
{
  return nsMath::Log2i(s_uiReflectionCubeMapSize) - 1; // only down to 4x4
}

//////////////////////////////////////////////////////////////////////////
/// nsReflectionPool::Data

struct nsReflectionPool::Data
{
  Data();
  ~Data();

  struct ProbeData
  {
    nsReflectionProbeDesc m_desc;
    nsTransform m_GlobalTransform;
    nsBitflags<nsProbeFlags> m_Flags;
    nsTextureCubeResourceHandle m_hCubeMap; // static data or empty for dynamic.
  };

  struct WorldReflectionData
  {
    WorldReflectionData()
      : m_mapping(s_uiNumReflectionProbeCubeMaps)
    {
    }
    NS_DISALLOW_COPY_AND_ASSIGN(WorldReflectionData);

    nsIdTable<nsReflectionProbeId, ProbeData> m_Probes;
    nsReflectionProbeId m_SkyLight; // SkyLight is always fixed at reflectionIndex 0.
    nsEventSubscriptionID m_mappingSubscriptionId = 0;
    nsReflectionProbeMapping m_mapping;
  };

  // WorldReflectionData management
  nsReflectionProbeId AddProbe(const nsWorld* pWorld, ProbeData&& probeData);
  nsReflectionPool::Data::WorldReflectionData& GetWorldData(const nsWorld* pWorld);
  void RemoveProbe(const nsWorld* pWorld, nsReflectionProbeId id);
  void UpdateProbeData(ProbeData& ref_probeData, const nsReflectionProbeDesc& desc, const nsReflectionProbeComponentBase* pComponent);
  bool UpdateSkyLightData(ProbeData& ref_probeData, const nsReflectionProbeDesc& desc, const nsSkyLightComponent* pComponent);
  void OnReflectionProbeMappingEvent(const nsUInt32 uiWorldIndex, const nsReflectionProbeMappingEvent& e);

  void PreExtraction();
  void PostExtraction();

  // Dynamic Update Queue (all worlds combined)
  nsHashSet<nsReflectionProbeRef> m_PendingDynamicUpdate;
  nsDeque<nsReflectionProbeRef> m_DynamicUpdateQueue;

  nsHashSet<nsReflectionProbeRef> m_ActiveDynamicUpdate;
  nsReflectionProbeUpdater m_ReflectionProbeUpdater;

  void CreateReflectionViewsAndResources();
  void CreateSkyIrradianceTexture();

  nsMutex m_Mutex;
  nsUInt64 m_uiWorldHasSkyLight = 0;
  nsUInt64 m_uiSkyIrradianceChanged = 0;
  nsHybridArray<nsUniquePtr<WorldReflectionData>, 2> m_WorldReflectionData;

  // GPU storage
  nsGALTextureHandle m_hFallbackReflectionSpecularTexture;
  nsGALTextureHandle m_hSkyIrradianceTexture;
  nsHybridArray<nsAmbientCube<nsColorLinear16f>, 64> m_SkyIrradianceStorage;

  // Debug data
  nsMeshResourceHandle m_hDebugSphere;
  nsHybridArray<nsMaterialResourceHandle, 6 * s_uiNumReflectionProbeCubeMaps> m_hDebugMaterial;
};
