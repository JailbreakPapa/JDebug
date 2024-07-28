#pragma once

#include <RendererCore/Declarations.h>

class nsDirectionalLightComponent;
class nsPointLightComponent;
class nsSpotLightComponent;
class nsGALTextureHandle;
class nsGALBufferHandle;
class nsView;
struct nsRenderWorldExtractionEvent;
struct nsRenderWorldRenderEvent;

class NS_RENDERERCORE_DLL nsShadowPool
{
public:
  static nsUInt32 AddDirectionalLight(const nsDirectionalLightComponent* pDirLight, const nsView* pReferenceView);
  static nsUInt32 AddPointLight(const nsPointLightComponent* pPointLight, float fScreenSpaceSize, const nsView* pReferenceView);
  static nsUInt32 AddSpotLight(const nsSpotLightComponent* pSpotLight, float fScreenSpaceSize, const nsView* pReferenceView);

  static nsGALTextureHandle GetShadowAtlasTexture();
  static nsGALBufferHandle GetShadowDataBuffer();

  /// \brief All exclude tags on this white list are copied from the reference views to the shadow views.
  static void AddExcludeTagToWhiteList(const nsTag& tag);

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererCore, ShadowPool);

  static void OnEngineStartup();
  static void OnEngineShutdown();

  static void OnExtractionEvent(const nsRenderWorldExtractionEvent& e);
  static void OnRenderEvent(const nsRenderWorldRenderEvent& e);

  struct Data;
  static Data* s_pData;
};
