#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Components/RenderTargetActivatorComponent.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsRenderTargetActivatorComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("RenderTarget", GetRenderTargetFile, SetRenderTargetFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Texture_Target", nsDependencyFlags::Package)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_COMPONENT_TYPE;
// clang-format on

nsRenderTargetActivatorComponent::nsRenderTargetActivatorComponent() = default;
nsRenderTargetActivatorComponent::~nsRenderTargetActivatorComponent() = default;

void nsRenderTargetActivatorComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  s << m_hRenderTarget;
}

void nsRenderTargetActivatorComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  nsStreamReader& s = inout_stream.GetStream();

  s >> m_hRenderTarget;
}

nsResult nsRenderTargetActivatorComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  if (m_hRenderTarget.IsValid())
  {
    ref_bounds = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3::MakeZero(), 0.1f);
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

void nsRenderTargetActivatorComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // only add render target views from main views
  // otherwise every shadow casting light source would activate a render target
  if (msg.m_pView->GetCameraUsageHint() != nsCameraUsageHint::MainView && msg.m_pView->GetCameraUsageHint() != nsCameraUsageHint::EditorView)
    return;

  if (!m_hRenderTarget.IsValid())
    return;

  nsResourceLock<nsRenderToTexture2DResource> pRenderTarget(m_hRenderTarget, nsResourceAcquireMode::BlockTillLoaded);

  for (auto hView : pRenderTarget->GetAllRenderViews())
  {
    nsRenderWorld::AddViewToRender(hView);
  }
}

void nsRenderTargetActivatorComponent::SetRenderTarget(const nsRenderToTexture2DResourceHandle& hResource)
{
  m_hRenderTarget = hResource;

  TriggerLocalBoundsUpdate();
}

void nsRenderTargetActivatorComponent::SetRenderTargetFile(const char* szFile)
{
  nsRenderToTexture2DResourceHandle hResource;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hResource = nsResourceManager::LoadResource<nsRenderToTexture2DResource>(szFile);
  }

  SetRenderTarget(hResource);
}

const char* nsRenderTargetActivatorComponent::GetRenderTargetFile() const
{
  if (!m_hRenderTarget.IsValid())
    return "";

  return m_hRenderTarget.GetResourceID();
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_RenderTargetActivatorComponent);
