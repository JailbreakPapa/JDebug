#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Lights/Implementation/ShadowPool.h>
#include <RendererCore/Lights/PointLightComponent.h>
#include <RendererCore/Pipeline/View.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsPointLightRenderData, 1, nsRTTIDefaultAllocator<nsPointLightRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_COMPONENT_TYPE(nsPointLightComponent, 2, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Range", GetRange, SetRange)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(0.0f), new nsSuffixAttribute(" m"), new nsMinValueTextAttribute("Auto")),
    //NS_ACCESSOR_PROPERTY("ProjectedTexture", GetProjectedTextureFile, SetProjectedTextureFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Texture_Cube")),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsSphereManipulatorAttribute("Range"),
    new nsPointLightVisualizerAttribute("Range", "Intensity", "LightColor"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsPointLightComponent::nsPointLightComponent()
{
  m_fEffectiveRange = CalculateEffectiveRange(m_fRange, m_fIntensity);
}

nsPointLightComponent::~nsPointLightComponent() = default;

nsResult nsPointLightComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  m_fEffectiveRange = CalculateEffectiveRange(m_fRange, m_fIntensity);

  ref_bounds = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3::MakeZero(), m_fEffectiveRange);
  return NS_SUCCESS;
}

void nsPointLightComponent::SetRange(float fRange)
{
  m_fRange = fRange;

  TriggerLocalBoundsUpdate();
}

float nsPointLightComponent::GetRange() const
{
  return m_fRange;
}

float nsPointLightComponent::GetEffectiveRange() const
{
  return m_fEffectiveRange;
}

// void nsPointLightComponent::SetProjectedTexture(const nsTextureCubeResourceHandle& hProjectedTexture)
//{
//   m_hProjectedTexture = hProjectedTexture;
//
//   InvalidateCachedRenderData();
// }
//
// const nsTextureCubeResourceHandle& nsPointLightComponent::GetProjectedTexture() const
//{
//   return m_hProjectedTexture;
// }

// void nsPointLightComponent::SetProjectedTextureFile(const char* szFile)
//{
//   nsTextureCubeResourceHandle hProjectedTexture;
//
//   if (!nsStringUtils::IsNullOrEmpty(szFile))
//   {
//     hProjectedTexture = nsResourceManager::LoadResource<nsTextureCubeResource>(szFile);
//   }
//
//   SetProjectedTexture(hProjectedTexture);
// }
//
// const char* nsPointLightComponent::GetProjectedTextureFile() const
//{
//   if (!m_hProjectedTexture.IsValid())
//     return "";
//
//   return m_hProjectedTexture.GetResourceID();
// }

void nsPointLightComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't extract light render data for selection or in shadow views.
  if (msg.m_OverrideCategory != nsInvalidRenderDataCategory || msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Shadow)
    return;

  if (m_fIntensity <= 0.0f || m_fEffectiveRange <= 0.0f)
    return;

  nsTransform t = GetOwner()->GetGlobalTransform();

  float fScreenSpaceSize = CalculateScreenSpaceSize(nsBoundingSphere::MakeFromCenterAndRadius(t.m_vPosition, m_fEffectiveRange * 0.5f), *msg.m_pView->GetCullingCamera());

  auto pRenderData = nsCreateRenderDataForThisFrame<nsPointLightRenderData>(GetOwner());

  pRenderData->m_GlobalTransform = t;
  pRenderData->m_LightColor = GetLightColor();
  pRenderData->m_fIntensity = m_fIntensity;
  pRenderData->m_fSpecularMultiplier = m_fSpecularMultiplier;
  pRenderData->m_fRange = m_fEffectiveRange;
  // pRenderData->m_hProjectedTexture = m_hProjectedTexture;
  pRenderData->m_uiShadowDataOffset = m_bCastShadows ? nsShadowPool::AddPointLight(this, fScreenSpaceSize, msg.m_pView) : nsInvalidIndex;

  pRenderData->FillBatchIdAndSortingKey(fScreenSpaceSize);

  nsRenderData::Caching::Enum caching = m_bCastShadows ? nsRenderData::Caching::Never : nsRenderData::Caching::IfStatic;
  msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::Light, caching);
}

void nsPointLightComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  nsTextureCubeResourceHandle m_hProjectedTexture;

  s << m_fRange;
  s << m_hProjectedTexture;
}

void nsPointLightComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  nsTextureCubeResourceHandle m_hProjectedTexture;

  s >> m_fRange;
  s >> m_hProjectedTexture;
}

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsPointLightVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsPointLightVisualizerAttribute>)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsPointLightVisualizerAttribute::nsPointLightVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
{
}

nsPointLightVisualizerAttribute::nsPointLightVisualizerAttribute(
  const char* szRangeProperty, const char* szIntensityProperty, const char* szColorProperty)
  : nsVisualizerAttribute(szRangeProperty, szIntensityProperty, szColorProperty)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsPointLightComponentPatch_1_2 : public nsGraphPatch
{
public:
  nsPointLightComponentPatch_1_2()
    : nsGraphPatch("nsPointLightComponent", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    ref_context.PatchBaseClass("nsLightComponent", 2, true);
  }
};

nsPointLightComponentPatch_1_2 g_nsPointLightComponentPatch_1_2;

NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_PointLightComponent);
