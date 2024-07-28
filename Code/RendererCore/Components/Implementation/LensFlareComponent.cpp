#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/SetColorMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Components/LensFlareComponent.h>
#include <RendererCore/Lights/DirectionalLightComponent.h>
#include <RendererCore/Lights/SpotLightComponent.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/Textures/Texture2DResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLensFlareRenderData, 1, nsRTTIDefaultAllocator<nsLensFlareRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsLensFlareRenderData::FillBatchIdAndSortingKey()
{
  // ignore upper 32 bit of the resource ID hash
  const nsUInt32 uiTextureIDHash = static_cast<nsUInt32>(m_hTexture.GetResourceIDHash());

  // Batch and sort by texture
  m_uiBatchId = uiTextureIDHash;
  m_uiSortingKey = uiTextureIDHash;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsLensFlareElement, nsNoBase, 1, nsRTTIDefaultAllocator<nsLensFlareElement>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Texture", GetTextureFile, SetTextureFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Texture_2D")),
    NS_MEMBER_PROPERTY("GreyscaleTexture", m_bGreyscaleTexture),
    NS_MEMBER_PROPERTY("Color", m_Color)->AddAttributes(new nsExposeColorAlphaAttribute()),
    NS_MEMBER_PROPERTY("ModulateByLightColor", m_bModulateByLightColor)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_MEMBER_PROPERTY("Size", m_fSize)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(10000.0f), new nsSuffixAttribute(" m")),
    NS_MEMBER_PROPERTY("MaxScreenSize", m_fMaxScreenSize)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f)),
    NS_MEMBER_PROPERTY("AspectRatio", m_fAspectRatio)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f)),
    NS_MEMBER_PROPERTY("ShiftToCenter", m_fShiftToCenter),
    NS_MEMBER_PROPERTY("InverseTonemap", m_bInverseTonemap),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

void nsLensFlareElement::SetTextureFile(const char* szFile)
{
  nsTexture2DResourceHandle hTexture;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hTexture = nsResourceManager::LoadResource<nsTexture2DResource>(szFile);
  }

  m_hTexture = hTexture;
}

const char* nsLensFlareElement::GetTextureFile() const
{
  if (!m_hTexture.IsValid())
    return "";

  return m_hTexture.GetResourceID();
}

nsResult nsLensFlareElement::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_hTexture;
  inout_stream << m_Color;
  inout_stream << m_fSize;
  inout_stream << m_fMaxScreenSize;
  inout_stream << m_fAspectRatio;
  inout_stream << m_fShiftToCenter;
  inout_stream << m_bInverseTonemap;
  inout_stream << m_bModulateByLightColor;
  inout_stream << m_bGreyscaleTexture;

  return NS_SUCCESS;
}

nsResult nsLensFlareElement::Deserialize(nsStreamReader& inout_stream)
{
  inout_stream >> m_hTexture;
  inout_stream >> m_Color;
  inout_stream >> m_fSize;
  inout_stream >> m_fMaxScreenSize;
  inout_stream >> m_fAspectRatio;
  inout_stream >> m_fShiftToCenter;
  inout_stream >> m_bInverseTonemap;
  inout_stream >> m_bModulateByLightColor;
  inout_stream >> m_bGreyscaleTexture;

  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsLensFlareComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("LinkToLightShape", GetLinkToLightShape, SetLinkToLightShape)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_MEMBER_PROPERTY("Intensity", m_fIntensity)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f)),
    NS_ACCESSOR_PROPERTY("OcclusionSampleRadius", GetOcclusionSampleRadius, SetOcclusionSampleRadius)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(0.1f), new nsSuffixAttribute(" m")),
    NS_MEMBER_PROPERTY("OcclusionSampleSpread", m_fOcclusionSampleSpread)->AddAttributes(new nsClampValueAttribute(0.0f, 1.0f), new nsDefaultValueAttribute(0.5f)),
    NS_MEMBER_PROPERTY("OcclusionDepthOffset", m_fOcclusionDepthOffset)->AddAttributes(new nsSuffixAttribute(" m")),
    NS_MEMBER_PROPERTY("ApplyFog", m_bApplyFog)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_ARRAY_MEMBER_PROPERTY("Elements", m_Elements)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
    new nsSphereManipulatorAttribute("OcclusionSampleRadius"),
    new nsSphereVisualizerAttribute("OcclusionSampleRadius", nsColor::White)
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

nsLensFlareComponent::nsLensFlareComponent() = default;
nsLensFlareComponent::~nsLensFlareComponent() = default;

void nsLensFlareComponent::OnActivated()
{
  SUPER::OnActivated();

  FindLightComponent();
}

void nsLensFlareComponent::OnDeactivated()
{
  SUPER::OnDeactivated();

  m_bDirectionalLight = false;
  m_hLightComponent.Invalidate();
}

void nsLensFlareComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  s.WriteArray(m_Elements).IgnoreResult();
  s << m_fIntensity;
  s << m_fOcclusionSampleRadius;
  s << m_fOcclusionSampleSpread;
  s << m_fOcclusionDepthOffset;
  s << m_bLinkToLightShape;
  s << m_bApplyFog;
}

void nsLensFlareComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);

  nsStreamReader& s = inout_stream.GetStream();

  s.ReadArray(m_Elements).IgnoreResult();
  s >> m_fIntensity;
  s >> m_fOcclusionSampleRadius;
  s >> m_fOcclusionSampleSpread;
  s >> m_fOcclusionDepthOffset;
  s >> m_bLinkToLightShape;
  s >> m_bApplyFog;
}

nsResult nsLensFlareComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  if (m_bDirectionalLight)
  {
    ref_bAlwaysVisible = true;
  }
  else
  {
    ref_bounds = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3::MakeZero(), m_fOcclusionSampleRadius);
  }
  return NS_SUCCESS;
}

void nsLensFlareComponent::SetLinkToLightShape(bool bLink)
{
  if (m_bLinkToLightShape == bLink)
    return;

  m_bLinkToLightShape = bLink;
  if (IsActiveAndInitialized())
  {
    FindLightComponent();
  }

  TriggerLocalBoundsUpdate();
}

void nsLensFlareComponent::SetOcclusionSampleRadius(float fRadius)
{
  m_fOcclusionSampleRadius = fRadius;

  TriggerLocalBoundsUpdate();
}

void nsLensFlareComponent::FindLightComponent()
{
  nsLightComponent* pLightComponent = nullptr;

  if (m_bLinkToLightShape)
  {
    nsGameObject* pObject = GetOwner();
    while (pObject != nullptr)
    {
      if (pObject->TryGetComponentOfBaseType(pLightComponent))
        break;

      pObject = pObject->GetParent();
    }
  }

  if (pLightComponent != nullptr)
  {
    m_bDirectionalLight = pLightComponent->IsInstanceOf<nsDirectionalLightComponent>();
    m_hLightComponent = pLightComponent->GetHandle();
  }
  else
  {
    m_bDirectionalLight = false;
    m_hLightComponent.Invalidate();
  }
}

void nsLensFlareComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't render in shadow views
  if (msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Shadow)
    return;

  // Don't extract render data for selection.
  if (msg.m_OverrideCategory != nsInvalidRenderDataCategory)
    return;

  if (m_fIntensity <= 0.0f)
    return;

  const nsCamera* pCamera = msg.m_pView->GetCamera();
  nsTransform globalTransform = GetOwner()->GetGlobalTransform();
  nsBoundingBoxSphere globalBounds = GetOwner()->GetGlobalBounds();
  float fScale = globalTransform.GetMaxScale();
  nsColor lightColor = nsColor::White;

  const nsLightComponent* pLightComponent = nullptr;
  if (GetWorld()->TryGetComponent(m_hLightComponent, pLightComponent))
  {
    lightColor = pLightComponent->GetLightColor();
    lightColor *= pLightComponent->GetIntensity() * 0.1f;
  }

  float fFade = 1.0f;
  if (auto pDirectionalLight = nsDynamicCast<const nsDirectionalLightComponent*>(pLightComponent))
  {
    nsTransform localOffset = nsTransform::MakeIdentity();
    localOffset.m_vPosition = nsVec3(pCamera->GetFarPlane() * -0.999, 0, 0);

    globalTransform = nsTransform::MakeGlobalTransform(globalTransform, localOffset);
    globalTransform.m_vPosition += pCamera->GetCenterPosition();

    if (pCamera->IsPerspective())
    {
      float fHalfHeight = nsMath::Tan(pCamera->GetFovY(1.0f) * 0.5f) * pCamera->GetFarPlane();
      fScale *= fHalfHeight;
    }

    lightColor *= 10.0f;
  }
  else if (auto pSpotLight = nsDynamicCast<const nsSpotLightComponent*>(pLightComponent))
  {
    const nsVec3 lightDir = globalTransform.TransformDirection(nsVec3::MakeAxisX());
    const nsVec3 cameraDir = (pCamera->GetCenterPosition() - globalTransform.m_vPosition).GetNormalized();

    const float cosAngle = lightDir.Dot(cameraDir);
    const float fCosInner = nsMath::Cos(pSpotLight->GetInnerSpotAngle() * 0.5f);
    const float fCosOuter = nsMath::Cos(pSpotLight->GetOuterSpotAngle() * 0.5f);
    fFade = nsMath::Saturate((cosAngle - fCosOuter) / nsMath::Max(0.001f, (fCosInner - fCosOuter)));
    fFade *= fFade;
  }

  for (auto& element : m_Elements)
  {
    if (element.m_hTexture.IsValid() == false)
      continue;

    nsColor color = element.m_Color * m_fIntensity;
    if (element.m_bModulateByLightColor)
    {
      color *= lightColor;
    }
    color.a = element.m_Color.a * fFade;

    if (color.GetLuminance() <= 0.0f || color.a <= 0.0f)
      continue;

    nsLensFlareRenderData* pRenderData = nsCreateRenderDataForThisFrame<nsLensFlareRenderData>(GetOwner());
    {
      pRenderData->m_GlobalTransform = globalTransform;
      pRenderData->m_GlobalBounds = globalBounds;
      pRenderData->m_hTexture = element.m_hTexture;
      pRenderData->m_Color = color.GetAsVec4();
      pRenderData->m_fSize = element.m_fSize * fScale;
      pRenderData->m_fMaxScreenSize = element.m_fMaxScreenSize * 2.0f;
      pRenderData->m_fAspectRatio = 1.0f / element.m_fAspectRatio;
      pRenderData->m_fShiftToCenter = element.m_fShiftToCenter;
      pRenderData->m_fOcclusionSampleRadius = m_fOcclusionSampleRadius * fScale;
      pRenderData->m_fOcclusionSampleSpread = m_fOcclusionSampleSpread;
      pRenderData->m_fOcclusionDepthOffset = m_fOcclusionDepthOffset * fScale;
      pRenderData->m_bInverseTonemap = element.m_bInverseTonemap;
      pRenderData->m_bGreyscaleTexture = element.m_bGreyscaleTexture;
      pRenderData->m_bApplyFog = m_bApplyFog;

      pRenderData->FillBatchIdAndSortingKey();
    }

    msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::LitTransparent,
      pLightComponent != nullptr ? nsRenderData::Caching::Never : nsRenderData::Caching::IfStatic);
  }
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_LensFlareComponent);
