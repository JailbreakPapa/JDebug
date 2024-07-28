#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Configuration/CVar.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Lights/Implementation/ShadowPool.h>
#include <RendererCore/Lights/SpotLightComponent.h>
#include <RendererCore/Pipeline/View.h>

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
nsCVarBool cvar_RenderingLightingVisScreenSpaceSize("Rendering.Lighting.VisScreenSpaceSize", false, nsCVarFlags::Default, "Enables debug visualization of light screen space size calculation");
#endif

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSpotLightRenderData, 1, nsRTTIDefaultAllocator<nsSpotLightRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_COMPONENT_TYPE(nsSpotLightComponent, 2, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Range", GetRange, SetRange)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(0.0f), new nsSuffixAttribute(" m"), new nsMinValueTextAttribute("Auto")),
    NS_ACCESSOR_PROPERTY("InnerSpotAngle", GetInnerSpotAngle, SetInnerSpotAngle)->AddAttributes(new nsClampValueAttribute(nsAngle::MakeFromDegree(0.0f), nsAngle::MakeFromDegree(179.0f)), new nsDefaultValueAttribute(nsAngle::MakeFromDegree(15.0f))),
    NS_ACCESSOR_PROPERTY("OuterSpotAngle", GetOuterSpotAngle, SetOuterSpotAngle)->AddAttributes(new nsClampValueAttribute(nsAngle::MakeFromDegree(0.0f), nsAngle::MakeFromDegree(179.0f)), new nsDefaultValueAttribute(nsAngle::MakeFromDegree(30.0f))),
    //NS_ACCESSOR_PROPERTY("ProjectedTexture", GetProjectedTextureFile, SetProjectedTextureFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Texture_2D")),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsSpotLightVisualizerAttribute("OuterSpotAngle", "Range", "Intensity", "LightColor"),
    new nsConeLengthManipulatorAttribute("Range"),
    new nsConeAngleManipulatorAttribute("OuterSpotAngle", 1.5f),
    new nsConeAngleManipulatorAttribute("InnerSpotAngle", 1.5f),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsSpotLightComponent::nsSpotLightComponent()
{
  m_fEffectiveRange = CalculateEffectiveRange(m_fRange, m_fIntensity);
}

nsSpotLightComponent::~nsSpotLightComponent() = default;

nsResult nsSpotLightComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  m_fEffectiveRange = CalculateEffectiveRange(m_fRange, m_fIntensity);

  ref_bounds = CalculateBoundingSphere(nsTransform::MakeIdentity(), m_fEffectiveRange);
  return NS_SUCCESS;
}

void nsSpotLightComponent::SetRange(float fRange)
{
  m_fRange = fRange;

  TriggerLocalBoundsUpdate();
}

float nsSpotLightComponent::GetRange() const
{
  return m_fRange;
}

float nsSpotLightComponent::GetEffectiveRange() const
{
  return m_fEffectiveRange;
}

void nsSpotLightComponent::SetInnerSpotAngle(nsAngle spotAngle)
{
  m_InnerSpotAngle = nsMath::Clamp(spotAngle, nsAngle::MakeFromDegree(0.0f), m_OuterSpotAngle);

  InvalidateCachedRenderData();
}

nsAngle nsSpotLightComponent::GetInnerSpotAngle() const
{
  return m_InnerSpotAngle;
}

void nsSpotLightComponent::SetOuterSpotAngle(nsAngle spotAngle)
{
  m_OuterSpotAngle = nsMath::Clamp(spotAngle, m_InnerSpotAngle, nsAngle::MakeFromDegree(179.0f));

  TriggerLocalBoundsUpdate();
}

nsAngle nsSpotLightComponent::GetOuterSpotAngle() const
{
  return m_OuterSpotAngle;
}

// void nsSpotLightComponent::SetProjectedTexture(const nsTexture2DResourceHandle& hProjectedTexture)
//{
//   m_hProjectedTexture = hProjectedTexture;
//
//   InvalidateCachedRenderData();
// }
//
// const nsTexture2DResourceHandle& nsSpotLightComponent::GetProjectedTexture() const
//{
//   return m_hProjectedTexture;
// }
//
// void nsSpotLightComponent::SetProjectedTextureFile(const char* szFile)
//{
//   nsTexture2DResourceHandle hProjectedTexture;
//
//   if (!nsStringUtils::IsNullOrEmpty(szFile))
//   {
//     hProjectedTexture = nsResourceManager::LoadResource<nsTexture2DResource>(szFile);
//   }
//
//   SetProjectedTexture(hProjectedTexture);
// }
//
// const char* nsSpotLightComponent::GetProjectedTextureFile() const
//{
//   if (!m_hProjectedTexture.IsValid())
//     return "";
//
//   return m_hProjectedTexture.GetResourceID();
// }

void nsSpotLightComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't extract light render data for selection or in shadow views.
  if (msg.m_OverrideCategory != nsInvalidRenderDataCategory || msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Shadow)
    return;

  if (m_fIntensity <= 0.0f || m_fEffectiveRange <= 0.0f || m_OuterSpotAngle.GetRadian() <= 0.0f)
    return;

  nsTransform t = GetOwner()->GetGlobalTransform();
  nsBoundingSphere bs = CalculateBoundingSphere(t, m_fEffectiveRange * 0.5f);

  float fScreenSpaceSize = CalculateScreenSpaceSize(bs, *msg.m_pView->GetCullingCamera());

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  if (cvar_RenderingLightingVisScreenSpaceSize)
  {
    nsStringBuilder sb;
    sb.SetFormat("{0}", fScreenSpaceSize);
    nsDebugRenderer::Draw3DText(msg.m_pView->GetHandle(), sb, t.m_vPosition, nsColor::Olive);
    nsDebugRenderer::DrawLineSphere(msg.m_pView->GetHandle(), bs, nsColor::Olive);
  }
#endif

  auto pRenderData = nsCreateRenderDataForThisFrame<nsSpotLightRenderData>(GetOwner());

  pRenderData->m_GlobalTransform = t;
  pRenderData->m_LightColor = GetLightColor();
  pRenderData->m_fIntensity = m_fIntensity;
  pRenderData->m_fSpecularMultiplier = m_fSpecularMultiplier;
  pRenderData->m_fRange = m_fEffectiveRange;
  pRenderData->m_InnerSpotAngle = m_InnerSpotAngle;
  pRenderData->m_OuterSpotAngle = m_OuterSpotAngle;
  // pRenderData->m_hProjectedTexture = m_hProjectedTexture;
  pRenderData->m_uiShadowDataOffset = m_bCastShadows ? nsShadowPool::AddSpotLight(this, fScreenSpaceSize, msg.m_pView) : nsInvalidIndex;

  pRenderData->FillBatchIdAndSortingKey(fScreenSpaceSize);

  nsRenderData::Caching::Enum caching = m_bCastShadows ? nsRenderData::Caching::Never : nsRenderData::Caching::IfStatic;
  msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::Light, caching);
}

void nsSpotLightComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  s << m_fRange;
  s << m_InnerSpotAngle;
  s << m_OuterSpotAngle;
  s << ""; // GetProjectedTextureFile();
}

void nsSpotLightComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  nsTexture2DResourceHandle m_hProjectedTexture;

  s >> m_fRange;
  s >> m_InnerSpotAngle;
  s >> m_OuterSpotAngle;

  nsStringBuilder temp;
  s >> temp;
  // SetProjectedTextureFile(temp);
}

nsBoundingSphere nsSpotLightComponent::CalculateBoundingSphere(const nsTransform& t, float fRange) const
{
  nsBoundingSphere res;
  nsAngle halfAngle = m_OuterSpotAngle / 2.0f;
  nsVec3 position = t.m_vPosition;
  nsVec3 forwardDir = t.m_qRotation * nsVec3(1.0f, 0.0f, 0.0f);

  if (halfAngle > nsAngle::MakeFromDegree(45.0f))
  {
    res.m_vCenter = position + nsMath::Cos(halfAngle) * fRange * forwardDir;
    res.m_fRadius = nsMath::Sin(halfAngle) * fRange;
  }
  else
  {
    res.m_fRadius = fRange / (2.0f * nsMath::Cos(halfAngle));
    res.m_vCenter = position + forwardDir * res.m_fRadius;
  }

  return res;
}

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSpotLightVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsSpotLightVisualizerAttribute>)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsSpotLightVisualizerAttribute::nsSpotLightVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
{
}

nsSpotLightVisualizerAttribute::nsSpotLightVisualizerAttribute(
  const char* szAngleProperty, const char* szRangeProperty, const char* szIntensityProperty, const char* szColorProperty)
  : nsVisualizerAttribute(szAngleProperty, szRangeProperty, szIntensityProperty, szColorProperty)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsSpotLightComponentPatch_1_2 : public nsGraphPatch
{
public:
  nsSpotLightComponentPatch_1_2()
    : nsGraphPatch("nsSpotLightComponent", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    ref_context.PatchBaseClass("nsLightComponent", 2, true);

    pNode->RenameProperty("Inner Spot Angle", "InnerSpotAngle");
    pNode->RenameProperty("Outer Spot Angle", "OuterSpotAngle");
  }
};

nsSpotLightComponentPatch_1_2 g_nsSpotLightComponentPatch_1_2;


NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_SpotLightComponent);
