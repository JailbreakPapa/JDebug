#include <RendererCore/RendererCorePCH.h>

#include <Core/GameApplication/GameApplicationBase.h>
#include <Core/Graphics/Camera.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Profiling/Profiling.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Lights/DirectionalLightComponent.h>
#include <RendererCore/AdvancedShadows/VirtualShadowMapSystem.h>
#include <RendererCore/Lights/PointLightComponent.h>
#include <RendererCore/Lights/SpotLightComponent.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Utils/CoreRenderProfile.h>
#include <RendererFoundation/CommandEncoder/RenderCommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/Pass.h>
#include <RendererFoundation/Resources/Texture.h>

#include <RendererCore/../../../Data/Base/Shaders/Common/LightData.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, SVSMPool)
  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core",
    "RenderWorld"
  END_SUBSYSTEM_DEPENDENCIES

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    nsSVSMPool::OnEngineStartup();
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    nsSVSMPool::OnEngineShutdown();
  }
NS_END_SUBSYSTEM_DECLARATION;
// clang-format on



static nsUInt32 s_uiLastConfigModification = 0;
static float s_fFadeOutScaleStart = 0.0f;
static float s_fFadeOutScaleEnd = 0.0f;

struct ShadowView
{
  nsViewHandle m_hView;
  nsCamera m_Camera;
};
/// TODO: Verify that SVSM's are Moved to their own file/class.



nsUInt32 nsSVSMPool::AddDirectionalLightPool(const nsDirectionalLightComponent* pDirLight, const nsView* pReferenceView)
{
  return nsUInt32();
}

nsGALTextureHandle* nsSVSMPool::GetRootClipSpaceTexture()
{
  return nullptr;
}

nsGALBufferHandle* nsSVSMPool::GetRootClipSpaceBuffer()
{
  return nullptr;
}
// static
void nsSVSMPool::OnEngineStartup()
{
}

void nsSVSMPool::OnEngineShutdown()
{
}

void nsSVSMPool::OnExtractionEvent(const nsRenderWorldExtractionEvent& e)
{
}

void nsSVSMPool::OnRenderEvent(const nsRenderWorldRenderEvent& e)
{
}
