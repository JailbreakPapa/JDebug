#pragma once

#include <Core/Graphics/AmbientCubeBasis.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/Declarations.h>

class nsGALTextureHandle;
class nsGALBufferHandle;
class nsView;
class nsWorld;
class nsComponent;
struct nsRenderWorldExtractionEvent;
struct nsRenderWorldRenderEvent;
struct nsMsgExtractRenderData;
struct nsReflectionProbeDesc;
class nsReflectionProbeRenderData;
using nsReflectionProbeId = nsGenericId<24, 8>;
class nsReflectionProbeComponentBase;
class nsSkyLightComponent;

class NS_RENDERERCORE_DLL nsReflectionPool
{
public:
  // Probes
  static nsReflectionProbeId RegisterReflectionProbe(const nsWorld* pWorld, const nsReflectionProbeDesc& desc, const nsReflectionProbeComponentBase* pComponent);
  static void DeregisterReflectionProbe(const nsWorld* pWorld, nsReflectionProbeId id);
  static void UpdateReflectionProbe(const nsWorld* pWorld, nsReflectionProbeId id, const nsReflectionProbeDesc& desc, const nsReflectionProbeComponentBase* pComponent);
  static void ExtractReflectionProbe(const nsComponent* pComponent, nsMsgExtractRenderData& ref_msg, nsReflectionProbeRenderData* pRenderData, const nsWorld* pWorld, nsReflectionProbeId id, float fPriority);

  // SkyLight
  static nsReflectionProbeId RegisterSkyLight(const nsWorld* pWorld, nsReflectionProbeDesc& ref_desc, const nsSkyLightComponent* pComponent);
  static void DeregisterSkyLight(const nsWorld* pWorld, nsReflectionProbeId id);
  static void UpdateSkyLight(const nsWorld* pWorld, nsReflectionProbeId id, const nsReflectionProbeDesc& desc, const nsSkyLightComponent* pComponent);


  static void SetConstantSkyIrradiance(const nsWorld* pWorld, const nsAmbientCube<nsColor>& skyIrradiance);
  static void ResetConstantSkyIrradiance(const nsWorld* pWorld);

  static nsUInt32 GetReflectionCubeMapSize();
  static nsGALTextureHandle GetReflectionSpecularTexture(nsUInt32 uiWorldIndex, nsEnum<nsCameraUsageHint> cameraUsageHint);
  static nsGALTextureHandle GetSkyIrradianceTexture();

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererCore, ReflectionPool);

  static void OnEngineStartup();
  static void OnEngineShutdown();

  static void OnExtractionEvent(const nsRenderWorldExtractionEvent& e);
  static void OnRenderEvent(const nsRenderWorldRenderEvent& e);

  struct Data;
  static Data* s_pData;
};
