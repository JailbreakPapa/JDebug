#include <RendererCore/RendererCorePCH.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Components/CameraComponent.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Pipeline/RenderPipelineResource.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Textures/Texture2DResource.h>


nsCameraComponentManager::nsCameraComponentManager(nsWorld* pWorld)
  : nsComponentManager<nsCameraComponent, nsBlockStorageType::Compact>(pWorld)
{
  nsRenderWorld::s_CameraConfigsModifiedEvent.AddEventHandler(nsMakeDelegate(&nsCameraComponentManager::OnCameraConfigsChanged, this));
}

nsCameraComponentManager::~nsCameraComponentManager()
{
  nsRenderWorld::s_CameraConfigsModifiedEvent.RemoveEventHandler(nsMakeDelegate(&nsCameraComponentManager::OnCameraConfigsChanged, this));
}

void nsCameraComponentManager::Initialize()
{
  auto desc = NS_CREATE_MODULE_UPDATE_FUNCTION_DESC(nsCameraComponentManager::Update, this);
  desc.m_Phase = UpdateFunctionDesc::Phase::PostTransform;

  this->RegisterUpdateFunction(desc);

  nsRenderWorld::s_ViewCreatedEvent.AddEventHandler(nsMakeDelegate(&nsCameraComponentManager::OnViewCreated, this));
}

void nsCameraComponentManager::Deinitialize()
{
  nsRenderWorld::s_ViewCreatedEvent.RemoveEventHandler(nsMakeDelegate(&nsCameraComponentManager::OnViewCreated, this));

  SUPER::Deinitialize();
}

void nsCameraComponentManager::Update(const nsWorldModule::UpdateContext& context)
{
  for (auto hCameraComponent : m_ModifiedCameras)
  {
    nsCameraComponent* pCameraComponent = nullptr;
    if (!TryGetComponent(hCameraComponent, pCameraComponent))
    {
      continue;
    }

    if (nsView* pView = nsRenderWorld::GetViewByUsageHint(pCameraComponent->GetUsageHint(), nsCameraUsageHint::None, GetWorld()))
    {
      pCameraComponent->ApplySettingsToView(pView);
    }

    pCameraComponent->m_bIsModified = false;
  }

  m_ModifiedCameras.Clear();

  for (auto hCameraComponent : m_RenderTargetCameras)
  {
    nsCameraComponent* pCameraComponent = nullptr;
    if (!TryGetComponent(hCameraComponent, pCameraComponent))
    {
      continue;
    }

    pCameraComponent->UpdateRenderTargetCamera();
  }

  for (auto it = GetComponents(); it.IsValid(); ++it)
  {
    if (it->IsActiveAndInitialized() && it->m_bShowStats && it->GetUsageHint() == nsCameraUsageHint::MainView)
    {
      if (nsView* pView = nsRenderWorld::GetViewByUsageHint(nsCameraUsageHint::MainView, nsCameraUsageHint::EditorView, GetWorld()))
      {
        it->ShowStats(pView);
      }
    }
  }
}

void nsCameraComponentManager::ReinitializeAllRenderTargetCameras()
{
  NS_LOCK(GetWorld()->GetWriteMarker());

  for (auto it = GetComponents(); it.IsValid(); ++it)
  {
    if (it->IsActiveAndInitialized())
    {
      it->DeactivateRenderToTexture();
      it->ActivateRenderToTexture();
    }
  }
}

const nsCameraComponent* nsCameraComponentManager::GetCameraByUsageHint(nsCameraUsageHint::Enum usageHint) const
{
  for (auto it = GetComponents(); it.IsValid(); ++it)
  {
    if (it->IsActiveAndInitialized() && it->GetUsageHint() == usageHint)
    {
      return it;
    }
  }

  return nullptr;
}

nsCameraComponent* nsCameraComponentManager::GetCameraByUsageHint(nsCameraUsageHint::Enum usageHint)
{
  for (auto it = GetComponents(); it.IsValid(); ++it)
  {
    if (it->IsActiveAndInitialized() && it->GetUsageHint() == usageHint)
    {
      return it;
    }
  }

  return nullptr;
}

void nsCameraComponentManager::AddRenderTargetCamera(nsCameraComponent* pComponent)
{
  m_RenderTargetCameras.PushBack(pComponent->GetHandle());
}

void nsCameraComponentManager::RemoveRenderTargetCamera(nsCameraComponent* pComponent)
{
  m_RenderTargetCameras.RemoveAndSwap(pComponent->GetHandle());
}

void nsCameraComponentManager::OnViewCreated(nsView* pView)
{
  // Mark all cameras as modified so the new view gets the proper settings
  for (auto it = GetComponents(); it.IsValid(); ++it)
  {
    it->MarkAsModified(this);
  }
}

void nsCameraComponentManager::OnCameraConfigsChanged(void* dummy)
{
  ReinitializeAllRenderTargetCameras();
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsCameraComponent, 10, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("EditorShortcut", m_iEditorShortcut)->AddAttributes(new nsDefaultValueAttribute(-1), new nsClampValueAttribute(-1, 9)),
    NS_ENUM_ACCESSOR_PROPERTY("UsageHint", nsCameraUsageHint, GetUsageHint, SetUsageHint),
    NS_ENUM_ACCESSOR_PROPERTY("Mode", nsCameraMode, GetCameraMode, SetCameraMode),
    NS_ACCESSOR_PROPERTY("RenderTarget", GetRenderTargetFile, SetRenderTargetFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Texture_Target", nsDependencyFlags::Package)),
    NS_ACCESSOR_PROPERTY("RenderTargetOffset", GetRenderTargetRectOffset, SetRenderTargetRectOffset)->AddAttributes(new nsClampValueAttribute(nsVec2(0.0f), nsVec2(0.9f))),
    NS_ACCESSOR_PROPERTY("RenderTargetSize", GetRenderTargetRectSize, SetRenderTargetRectSize)->AddAttributes(new nsDefaultValueAttribute(nsVec2(1.0f)), new nsClampValueAttribute(nsVec2(0.1f), nsVec2(1.0f))),
    NS_ACCESSOR_PROPERTY("NearPlane", GetNearPlane, SetNearPlane)->AddAttributes(new nsDefaultValueAttribute(0.25f), new nsClampValueAttribute(0.01f, 4.0f)),
    NS_ACCESSOR_PROPERTY("FarPlane", GetFarPlane, SetFarPlane)->AddAttributes(new nsDefaultValueAttribute(1000.0f), new nsClampValueAttribute(5.0, 10000.0f)),
    NS_ACCESSOR_PROPERTY("FOV", GetFieldOfView, SetFieldOfView)->AddAttributes(new nsDefaultValueAttribute(60.0f), new nsClampValueAttribute(1.0f, 170.0f)),
    NS_ACCESSOR_PROPERTY("Dimensions", GetOrthoDimension, SetOrthoDimension)->AddAttributes(new nsDefaultValueAttribute(10.0f), new nsClampValueAttribute(0.01f, 10000.0f)),
    NS_SET_MEMBER_PROPERTY("IncludeTags", m_IncludeTags)->AddAttributes(new nsTagSetWidgetAttribute("Default")),
    NS_SET_MEMBER_PROPERTY("ExcludeTags", m_ExcludeTags)->AddAttributes(new nsTagSetWidgetAttribute("Default")),
    NS_ACCESSOR_PROPERTY("CameraRenderPipeline", GetRenderPipelineEnum, SetRenderPipelineEnum)->AddAttributes(new nsDynamicStringEnumAttribute("CameraPipelines")),
    NS_ACCESSOR_PROPERTY("Aperture", GetAperture, SetAperture)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(1.0f, 32.0f), new nsSuffixAttribute(" f-stop(s)")),
    NS_ACCESSOR_PROPERTY("ShutterTime", GetShutterTime, SetShutterTime)->AddAttributes(new nsDefaultValueAttribute(nsTime::MakeFromSeconds(1.0)), new nsClampValueAttribute(nsTime::MakeFromSeconds(1.0f / 100000.0f), nsTime::MakeFromSeconds(600.0f))),
    NS_ACCESSOR_PROPERTY("ISO", GetISO, SetISO)->AddAttributes(new nsDefaultValueAttribute(100.0f), new nsClampValueAttribute(50.0f, 64000.0f)),
    NS_ACCESSOR_PROPERTY("ExposureCompensation", GetExposureCompensation, SetExposureCompensation)->AddAttributes(new nsClampValueAttribute(-32.0f, 32.0f)),
    NS_MEMBER_PROPERTY("ShowStats", m_bShowStats),
    //NS_ACCESSOR_PROPERTY_READ_ONLY("EV100", GetEV100),
    //NS_ACCESSOR_PROPERTY_READ_ONLY("FinalExposure", GetExposure),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
    new nsDirectionVisualizerAttribute(nsBasisAxis::PositiveX, 1.0f, nsColor::DarkSlateBlue),
    new nsCameraVisualizerAttribute("Mode", "FOV", "Dimensions", "NearPlane", "FarPlane"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsCameraComponent::nsCameraComponent() = default;
nsCameraComponent::~nsCameraComponent() = default;

void nsCameraComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s << m_UsageHint.GetValue();
  s << m_Mode.GetValue();
  s << m_fNearPlane;
  s << m_fFarPlane;
  s << m_fPerspectiveFieldOfView;
  s << m_fOrthoDimension;

  // Version 2 till 7
  // s << m_hRenderPipeline;

  // Version 3
  s << m_fAperture;
  s << static_cast<float>(m_ShutterTime.GetSeconds());
  s << m_fISO;
  s << m_fExposureCompensation;

  // Version 4
  m_IncludeTags.Save(s);
  m_ExcludeTags.Save(s);

  // Version 6
  s << m_hRenderTarget;

  // Version 7
  s << m_vRenderTargetRectOffset;
  s << m_vRenderTargetRectSize;

  // Version 8
  s << m_sRenderPipeline;

  // Version 10
  s << m_bShowStats;
}

void nsCameraComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  auto& s = inout_stream.GetStream();

  nsCameraUsageHint::StorageType usage;
  s >> usage;
  if (uiVersion == 1 && usage > nsCameraUsageHint::MainView)
    usage = nsCameraUsageHint::None;
  m_UsageHint.SetValue(usage);

  nsCameraMode::StorageType cam;
  s >> cam;
  m_Mode.SetValue(cam);

  s >> m_fNearPlane;
  s >> m_fFarPlane;
  s >> m_fPerspectiveFieldOfView;
  s >> m_fOrthoDimension;

  if (uiVersion >= 2 && uiVersion <= 7)
  {
    nsRenderPipelineResourceHandle m_hRenderPipeline;
    s >> m_hRenderPipeline;
  }

  if (uiVersion >= 3)
  {
    s >> m_fAperture;
    float shutterTime;
    s >> shutterTime;
    m_ShutterTime = nsTime::MakeFromSeconds(shutterTime);
    s >> m_fISO;
    s >> m_fExposureCompensation;
  }

  if (uiVersion >= 4)
  {
    m_IncludeTags.Load(s, nsTagRegistry::GetGlobalRegistry());
    m_ExcludeTags.Load(s, nsTagRegistry::GetGlobalRegistry());
  }

  if (uiVersion >= 6)
  {
    s >> m_hRenderTarget;
  }

  if (uiVersion >= 7)
  {
    s >> m_vRenderTargetRectOffset;
    s >> m_vRenderTargetRectSize;
  }

  if (uiVersion >= 8)
  {
    s >> m_sRenderPipeline;
  }

  if (uiVersion >= 10)
  {
    s >> m_bShowStats;
  }

  MarkAsModified();
}

void nsCameraComponent::UpdateRenderTargetCamera()
{
  if (!m_bRenderTargetInitialized)
    return;

  // recreate everything, if the view got invalidated in between
  if (m_hRenderTargetView.IsInvalidated())
  {
    DeactivateRenderToTexture();
    ActivateRenderToTexture();
  }

  nsView* pView = nullptr;
  if (!nsRenderWorld::TryGetView(m_hRenderTargetView, pView))
    return;

  ApplySettingsToView(pView);

  if (m_Mode == nsCameraMode::PerspectiveFixedFovX || m_Mode == nsCameraMode::PerspectiveFixedFovY)
    m_RenderTargetCamera.SetCameraMode(GetCameraMode(), m_fPerspectiveFieldOfView, m_fNearPlane, m_fFarPlane);
  else
    m_RenderTargetCamera.SetCameraMode(GetCameraMode(), m_fOrthoDimension, m_fNearPlane, m_fFarPlane);

  m_RenderTargetCamera.LookAt(
    GetOwner()->GetGlobalPosition(), GetOwner()->GetGlobalPosition() + GetOwner()->GetGlobalDirForwards(), GetOwner()->GetGlobalDirUp());
}

void nsCameraComponent::ShowStats(nsView* pView)
{
  if (!m_bShowStats)
    return;

  // draw stats
  {
    const nsStringView sName = GetOwner()->GetName();

    nsStringBuilder sb;
    sb.SetFormat("Camera '{0}':\nEV100: {1}, Exposure: {2}", sName.IsEmpty() ? pView->GetName() : sName, GetEV100(), GetExposure());
    nsDebugRenderer::DrawInfoText(pView->GetHandle(), nsDebugTextPlacement::TopLeft, "CamStats", sb, nsColor::White);
  }

  // draw frustum
  {
    const nsGameObject* pOwner = GetOwner();
    nsVec3 vPosition = pOwner->GetGlobalPosition();
    nsVec3 vForward = pOwner->GetGlobalDirForwards();
    nsVec3 vUp = pOwner->GetGlobalDirUp();

    const nsMat4 viewMatrix = nsGraphicsUtils::CreateLookAtViewMatrix(vPosition, vPosition + vForward, vUp);

    nsMat4 projectionMatrix = pView->GetProjectionMatrix(nsCameraEye::Left); // todo: Stereo support
    nsMat4 viewProjectionMatrix = projectionMatrix * viewMatrix;

    nsFrustum frustum = nsFrustum::MakeFromMVP(viewProjectionMatrix);

    // TODO: limit far plane to 10 meters

    nsDebugRenderer::DrawLineFrustum(GetWorld(), frustum, nsColor::LimeGreen);
  }
}

void nsCameraComponent::SetUsageHint(nsEnum<nsCameraUsageHint> val)
{
  if (val == m_UsageHint)
    return;

  DeactivateRenderToTexture();

  m_UsageHint = val;

  ActivateRenderToTexture();

  MarkAsModified();
}

void nsCameraComponent::SetRenderTargetFile(const char* szFile)
{
  DeactivateRenderToTexture();

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    m_hRenderTarget = nsResourceManager::LoadResource<nsRenderToTexture2DResource>(szFile);
  }
  else
  {
    m_hRenderTarget.Invalidate();
  }

  ActivateRenderToTexture();

  MarkAsModified();
}

const char* nsCameraComponent::GetRenderTargetFile() const
{
  if (!m_hRenderTarget.IsValid())
    return "";

  return m_hRenderTarget.GetResourceID();
}

void nsCameraComponent::SetRenderTargetRectOffset(nsVec2 value)
{
  DeactivateRenderToTexture();

  m_vRenderTargetRectOffset.x = nsMath::Clamp(value.x, 0.0f, 0.9f);
  m_vRenderTargetRectOffset.y = nsMath::Clamp(value.y, 0.0f, 0.9f);

  ActivateRenderToTexture();
}

void nsCameraComponent::SetRenderTargetRectSize(nsVec2 value)
{
  DeactivateRenderToTexture();

  m_vRenderTargetRectSize.x = nsMath::Clamp(value.x, 0.1f, 1.0f);
  m_vRenderTargetRectSize.y = nsMath::Clamp(value.y, 0.1f, 1.0f);

  ActivateRenderToTexture();
}

void nsCameraComponent::SetCameraMode(nsEnum<nsCameraMode> val)
{
  if (val == m_Mode)
    return;
  m_Mode = val;

  MarkAsModified();
}


void nsCameraComponent::SetNearPlane(float fVal)
{
  if (fVal == m_fNearPlane)
    return;
  m_fNearPlane = fVal;

  MarkAsModified();
}


void nsCameraComponent::SetFarPlane(float fVal)
{
  if (fVal == m_fFarPlane)
    return;
  m_fFarPlane = fVal;

  MarkAsModified();
}


void nsCameraComponent::SetFieldOfView(float fVal)
{
  if (fVal == m_fPerspectiveFieldOfView)
    return;
  m_fPerspectiveFieldOfView = fVal;

  MarkAsModified();
}


void nsCameraComponent::SetOrthoDimension(float fVal)
{
  if (fVal == m_fOrthoDimension)
    return;
  m_fOrthoDimension = fVal;

  MarkAsModified();
}

nsRenderPipelineResourceHandle nsCameraComponent::GetRenderPipeline() const
{
  return m_hCachedRenderPipeline;
}

nsViewHandle nsCameraComponent::GetRenderTargetView() const
{
  return m_hRenderTargetView;
}

const char* nsCameraComponent::GetRenderPipelineEnum() const
{
  return m_sRenderPipeline.GetData();
}

void nsCameraComponent::SetRenderPipelineEnum(const char* szFile)
{
  DeactivateRenderToTexture();

  m_sRenderPipeline.Assign(szFile);

  ActivateRenderToTexture();

  MarkAsModified();
}

void nsCameraComponent::SetAperture(float fAperture)
{
  if (m_fAperture == fAperture)
    return;
  m_fAperture = fAperture;

  MarkAsModified();
}

void nsCameraComponent::SetShutterTime(nsTime shutterTime)
{
  if (m_ShutterTime == shutterTime)
    return;
  m_ShutterTime = shutterTime;

  MarkAsModified();
}

void nsCameraComponent::SetISO(float fISO)
{
  if (m_fISO == fISO)
    return;
  m_fISO = fISO;

  MarkAsModified();
}

void nsCameraComponent::SetExposureCompensation(float fEC)
{
  if (m_fExposureCompensation == fEC)
    return;
  m_fExposureCompensation = fEC;

  MarkAsModified();
}

float nsCameraComponent::GetEV100() const
{
  // From: course_notes_moving_frostbite_to_pbr.pdf
  // EV number is defined as:
  // 2^ EV_s = N^2 / t and EV_s = EV_100 + log2 (S /100)
  // This gives
  // EV_s = log2 (N^2 / t)
  // EV_100 + log2 (S /100) = log2 (N^2 / t)
  // EV_100 = log2 (N^2 / t) - log2 (S /100)
  // EV_100 = log2 (N^2 / t . 100 / S)
  return nsMath::Log2((m_fAperture * m_fAperture) / m_ShutterTime.AsFloatInSeconds() * 100.0f / m_fISO) - m_fExposureCompensation;
}

float nsCameraComponent::GetExposure() const
{
  // Compute the maximum luminance possible with H_sbs sensitivity
  // maxLum = 78 / ( S * q ) * N^2 / t
  // = 78 / ( S * q ) * 2^ EV_100
  // = 78 / (100 * 0.65) * 2^ EV_100
  // = 1.2 * 2^ EV
  // Reference : http://en.wikipedia.org/wiki/Film_speed
  float maxLuminance = 1.2f * nsMath::Pow2(GetEV100());
  return 1.0f / maxLuminance;
}

void nsCameraComponent::ApplySettingsToView(nsView* pView) const
{
  if (m_UsageHint == nsCameraUsageHint::None)
    return;

  float fFovOrDim = m_fPerspectiveFieldOfView;
  if (m_Mode == nsCameraMode::OrthoFixedWidth || m_Mode == nsCameraMode::OrthoFixedHeight)
  {
    fFovOrDim = m_fOrthoDimension;
  }

  nsCamera* pCamera = pView->GetCamera();
  pCamera->SetCameraMode(m_Mode, fFovOrDim, m_fNearPlane, nsMath::Max(m_fNearPlane + 0.00001f, m_fFarPlane));
  pCamera->SetExposure(GetExposure());

  pView->m_IncludeTags = m_IncludeTags;
  pView->m_ExcludeTags = m_ExcludeTags;

  const nsTag& tagEditor = nsTagRegistry::GetGlobalRegistry().RegisterTag("Editor");
  pView->m_ExcludeTags.Set(tagEditor);

  if (m_hCachedRenderPipeline.IsValid())
  {
    pView->SetRenderPipelineResource(m_hCachedRenderPipeline);
  }
}

void nsCameraComponent::ResourceChangeEventHandler(const nsResourceEvent& e)
{
  switch (e.m_Type)
  {
    case nsResourceEvent::Type::ResourceExists:
    case nsResourceEvent::Type::ResourceCreated:
      return;

    case nsResourceEvent::Type::ResourceDeleted:
    case nsResourceEvent::Type::ResourceContentUnloading:
    case nsResourceEvent::Type::ResourceContentUpdated:
      // triggers a recreation of the view
      nsRenderWorld::DeleteView(m_hRenderTargetView);
      m_hRenderTargetView.Invalidate();
      break;

    default:
      break;
  }
}

void nsCameraComponent::MarkAsModified()
{
  if (!m_bIsModified)
  {
    GetWorld()->GetComponentManager<nsCameraComponentManager>()->m_ModifiedCameras.PushBack(GetHandle());
    m_bIsModified = true;
  }
}


void nsCameraComponent::MarkAsModified(nsCameraComponentManager* pCameraManager)
{
  if (!m_bIsModified)
  {
    pCameraManager->m_ModifiedCameras.PushBack(GetHandle());
    m_bIsModified = true;
  }
}

void nsCameraComponent::ActivateRenderToTexture()
{
  if (m_UsageHint != nsCameraUsageHint::RenderTarget)
    return;

  if (m_bRenderTargetInitialized || !m_hRenderTarget.IsValid() || m_sRenderPipeline.IsEmpty() || !IsActiveAndInitialized())
    return;

  nsResourceLock<nsRenderToTexture2DResource> pRenderTarget(m_hRenderTarget, nsResourceAcquireMode::BlockTillLoaded_NeverFail);

  if (pRenderTarget.GetAcquireResult() != nsResourceAcquireResult::Final)
  {
    return;
  }

  // query the render pipeline to use
  if (const auto* pConfig = nsRenderWorld::FindCameraConfig(m_sRenderPipeline))
  {
    m_hCachedRenderPipeline = pConfig->m_hRenderPipeline;
  }

  if (!m_hCachedRenderPipeline.IsValid())
    return;

  m_bRenderTargetInitialized = true;

  NS_ASSERT_DEV(m_hRenderTargetView.IsInvalidated(), "Render target view is already created");

  nsStringBuilder name;
  name.SetFormat("Camera RT: {0}", GetOwner()->GetName());

  nsView* pRenderTargetView = nullptr;
  m_hRenderTargetView = nsRenderWorld::CreateView(name, pRenderTargetView);

  pRenderTargetView->SetRenderPipelineResource(m_hCachedRenderPipeline);

  pRenderTargetView->SetWorld(GetWorld());
  pRenderTargetView->SetCamera(&m_RenderTargetCamera);

  pRenderTarget->m_ResourceEvents.AddEventHandler(nsMakeDelegate(&nsCameraComponent::ResourceChangeEventHandler, this));

  nsGALRenderTargets renderTargets;
  renderTargets.m_hRTs[0] = pRenderTarget->GetGALTexture();
  pRenderTargetView->SetRenderTargets(renderTargets);

  const float maxSizeX = 1.0f - m_vRenderTargetRectOffset.x;
  const float maxSizeY = 1.0f - m_vRenderTargetRectOffset.y;

  const float resX = (float)pRenderTarget->GetWidth();
  const float resY = (float)pRenderTarget->GetHeight();

  const float width = resX * nsMath::Min(maxSizeX, m_vRenderTargetRectSize.x);
  const float height = resY * nsMath::Min(maxSizeY, m_vRenderTargetRectSize.y);

  const float offsetX = m_vRenderTargetRectOffset.x * resX;
  const float offsetY = m_vRenderTargetRectOffset.y * resY;

  pRenderTargetView->SetViewport(nsRectFloat(offsetX, offsetY, width, height));

  pRenderTarget->AddRenderView(m_hRenderTargetView);

  GetWorld()->GetComponentManager<nsCameraComponentManager>()->AddRenderTargetCamera(this);
}

void nsCameraComponent::DeactivateRenderToTexture()
{
  if (!m_bRenderTargetInitialized)
    return;

  m_bRenderTargetInitialized = false;
  m_hCachedRenderPipeline.Invalidate();

  NS_ASSERT_DEBUG(m_hRenderTarget.IsValid(), "Render Target should be valid");

  if (m_hRenderTarget.IsValid())
  {
    nsResourceLock<nsRenderToTexture2DResource> pRenderTarget(m_hRenderTarget, nsResourceAcquireMode::BlockTillLoaded);
    pRenderTarget->RemoveRenderView(m_hRenderTargetView);

    pRenderTarget->m_ResourceEvents.RemoveEventHandler(nsMakeDelegate(&nsCameraComponent::ResourceChangeEventHandler, this));
  }

  if (!m_hRenderTargetView.IsInvalidated())
  {
    nsRenderWorld::DeleteView(m_hRenderTargetView);
    m_hRenderTargetView.Invalidate();
  }

  GetWorld()->GetComponentManager<nsCameraComponentManager>()->RemoveRenderTargetCamera(this);
}

void nsCameraComponent::OnActivated()
{
  SUPER::OnActivated();

  ActivateRenderToTexture();
}

void nsCameraComponent::OnDeactivated()
{
  DeactivateRenderToTexture();

  SUPER::OnDeactivated();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsCameraComponentPatch_4_5 : public nsGraphPatch
{
public:
  nsCameraComponentPatch_4_5()
    : nsGraphPatch("nsCameraComponent", 5)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    pNode->RenameProperty("Usage Hint", "UsageHint");
    pNode->RenameProperty("Near Plane", "NearPlane");
    pNode->RenameProperty("Far Plane", "FarPlane");
    pNode->RenameProperty("Include Tags", "IncludeTags");
    pNode->RenameProperty("Exclude Tags", "ExcludeTags");
    pNode->RenameProperty("Render Pipeline", "RenderPipeline");
    pNode->RenameProperty("Shutter Time", "ShutterTime");
    pNode->RenameProperty("Exposure Compensation", "ExposureCompensation");
  }
};

nsCameraComponentPatch_4_5 g_nsCameraComponentPatch_4_5;

//////////////////////////////////////////////////////////////////////////

class nsCameraComponentPatch_8_9 : public nsGraphPatch
{
public:
  nsCameraComponentPatch_8_9()
    : nsGraphPatch("nsCameraComponent", 9)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    // convert the "ShutterTime" property from float to nsTime
    if (auto pProp = pNode->FindProperty("ShutterTime"))
    {
      if (pProp->m_Value.IsA<float>())
      {
        const float shutterTime = pProp->m_Value.Get<float>();
        pProp->m_Value = nsTime::MakeFromSeconds(shutterTime);
      }
    }
  }
};

nsCameraComponentPatch_8_9 g_nsCameraComponentPatch_8_9;


NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_CameraComponent);
