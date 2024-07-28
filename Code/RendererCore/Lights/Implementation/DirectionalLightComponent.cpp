#include <RendererCore/RendererCorePCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Lights/DirectionalLightComponent.h>
#include <RendererCore/Lights/Implementation/ShadowPool.h>
#include <RendererCore/Pipeline/View.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDirectionalLightRenderData, 1, nsRTTIDefaultAllocator<nsDirectionalLightRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_COMPONENT_TYPE(nsDirectionalLightComponent, 3, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("NumCascades", GetNumCascades, SetNumCascades)->AddAttributes(new nsClampValueAttribute(1, 4), new nsDefaultValueAttribute(2)),
    NS_ACCESSOR_PROPERTY("MinShadowRange", GetMinShadowRange, SetMinShadowRange)->AddAttributes(new nsClampValueAttribute(0.1f, nsVariant()), new nsDefaultValueAttribute(30.0f), new nsSuffixAttribute(" m")),
    NS_ACCESSOR_PROPERTY("FadeOutStart", GetFadeOutStart, SetFadeOutStart)->AddAttributes(new nsClampValueAttribute(0.6f, 1.0f), new nsDefaultValueAttribute(0.8f)),
    NS_ACCESSOR_PROPERTY("SplitModeWeight", GetSplitModeWeight, SetSplitModeWeight)->AddAttributes(new nsClampValueAttribute(0.0f, 1.0f), new nsDefaultValueAttribute(0.7f)),
    NS_ACCESSOR_PROPERTY("NearPlaneOffset", GetNearPlaneOffset, SetNearPlaneOffset)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(100.0f), new nsSuffixAttribute(" m")),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsDirectionVisualizerAttribute(nsBasisAxis::PositiveX, 1.0f, nsColor::White, "LightColor"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsDirectionalLightComponent::nsDirectionalLightComponent() = default;
nsDirectionalLightComponent::~nsDirectionalLightComponent() = default;

nsResult nsDirectionalLightComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  ref_bAlwaysVisible = true;
  return NS_SUCCESS;
}

void nsDirectionalLightComponent::SetNumCascades(nsUInt32 uiNumCascades)
{
  m_uiNumCascades = nsMath::Clamp(uiNumCascades, 1u, 4u);

  InvalidateCachedRenderData();
}

nsUInt32 nsDirectionalLightComponent::GetNumCascades() const
{
  return m_uiNumCascades;
}

void nsDirectionalLightComponent::SetMinShadowRange(float fMinShadowRange)
{
  m_fMinShadowRange = nsMath::Max(fMinShadowRange, 0.0f);

  InvalidateCachedRenderData();
}

float nsDirectionalLightComponent::GetMinShadowRange() const
{
  return m_fMinShadowRange;
}

void nsDirectionalLightComponent::SetFadeOutStart(float fFadeOutStart)
{
  m_fFadeOutStart = nsMath::Clamp(fFadeOutStart, 0.0f, 1.0f);

  InvalidateCachedRenderData();
}

float nsDirectionalLightComponent::GetFadeOutStart() const
{
  return m_fFadeOutStart;
}

void nsDirectionalLightComponent::SetSplitModeWeight(float fSplitModeWeight)
{
  m_fSplitModeWeight = nsMath::Clamp(fSplitModeWeight, 0.0f, 1.0f);

  InvalidateCachedRenderData();
}

float nsDirectionalLightComponent::GetSplitModeWeight() const
{
  return m_fSplitModeWeight;
}

void nsDirectionalLightComponent::SetNearPlaneOffset(float fNearPlaneOffset)
{
  m_fNearPlaneOffset = nsMath::Max(fNearPlaneOffset, 0.0f);

  InvalidateCachedRenderData();
}

float nsDirectionalLightComponent::GetNearPlaneOffset() const
{
  return m_fNearPlaneOffset;
}

void nsDirectionalLightComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't extract light render data for selection or in shadow views.
  if (msg.m_OverrideCategory != nsInvalidRenderDataCategory || msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Shadow)
    return;

  if (m_fIntensity <= 0.0f)
    return;

  auto pRenderData = nsCreateRenderDataForThisFrame<nsDirectionalLightRenderData>(GetOwner());

  pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
  pRenderData->m_LightColor = GetLightColor();
  pRenderData->m_fIntensity = m_fIntensity;
  pRenderData->m_fSpecularMultiplier = m_fSpecularMultiplier;
  pRenderData->m_uiShadowDataOffset = m_bCastShadows ? nsShadowPool::AddDirectionalLight(this, msg.m_pView) : nsInvalidIndex;

  pRenderData->FillBatchIdAndSortingKey(1.0f);

  nsRenderData::Caching::Enum caching = m_bCastShadows ? nsRenderData::Caching::Never : nsRenderData::Caching::IfStatic;
  msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::Light, caching);
}

void nsDirectionalLightComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  s << m_uiNumCascades;
  s << m_fMinShadowRange;
  s << m_fFadeOutStart;
  s << m_fSplitModeWeight;
  s << m_fNearPlaneOffset;
}

void nsDirectionalLightComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  if (uiVersion >= 3)
  {
    s >> m_uiNumCascades;
    s >> m_fMinShadowRange;
    s >> m_fFadeOutStart;
    s >> m_fSplitModeWeight;
    s >> m_fNearPlaneOffset;
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsDirectionalLightComponentPatch_1_2 : public nsGraphPatch
{
public:
  nsDirectionalLightComponentPatch_1_2()
    : nsGraphPatch("nsDirectionalLightComponent", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    ref_context.PatchBaseClass("nsLightComponent", 2, true);
  }
};

nsDirectionalLightComponentPatch_1_2 g_nsDirectionalLightComponentPatch_1_2;



NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_DirectionalLightComponent);
