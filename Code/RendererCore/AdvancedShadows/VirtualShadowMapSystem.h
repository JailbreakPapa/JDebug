#pragma once
#include <RendererCore/Declarations.h>
#include <RendererCore/Lights/Implementation/ShadowPool.h>
/// @brief This class is a pool of Virtual Shadow Maps that are used to render shadows for the scene.
/// REFERENCES: https://ktstephano.github.io/rendering/stratusgfx/svsm https://github.com/TheRealMJP/Shadows https://silverspaceship.com/src/svt/

class nsDirectionalLightComponent;
class nsGALTextureHandle;
class nsGALBufferHandle;
class nsView;
struct nsRenderWorldExtractionEvent;
struct nsRenderWorldRenderEvent;
/// NOTE: Virtual Shadow Maps Will only work on Directional (Sun) Lights. Other Light types can be easily added and used, but for now directional lights will be the easiest to add.
class NS_RENDERERCORE_DLL nsSVSMPool
{
public:

  static nsUInt32 AddDirectionalLightPool(const nsDirectionalLightComponent* pDirLight, const nsView* pReferenceView);

  static nsGALTextureHandle* GetRootClipSpaceTexture();
  static nsGALBufferHandle* GetRootClipSpaceBuffer();

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererCore, SVSMPool);

  static void OnEngineStartup();
  static void OnEngineShutdown();

  static void OnExtractionEvent(const nsRenderWorldExtractionEvent& e);
  static void OnRenderEvent(const nsRenderWorldRenderEvent& e);
  struct Data;
  static Data* s_pData;
};
