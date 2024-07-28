#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/BoxReflectionProbeComponent.h>

#include <../../Data/Base/Shaders/Common/LightData.h>
#include <Core/Messages/TransformChangedMessage.h>
#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Pipeline/View.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsBoxReflectionProbeComponent, 2, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Extents", GetExtents, SetExtents)->AddAttributes(new nsClampValueAttribute(nsVec3(0.0f), {}), new nsDefaultValueAttribute(nsVec3(5.0f))),
    NS_ACCESSOR_PROPERTY("InfluenceScale", GetInfluenceScale, SetInfluenceScale)->AddAttributes(new nsClampValueAttribute(nsVec3(0.0f), nsVec3(1.0f)), new nsDefaultValueAttribute(nsVec3(1.0f))),
    NS_ACCESSOR_PROPERTY("InfluenceShift", GetInfluenceShift, SetInfluenceShift)->AddAttributes(new nsClampValueAttribute(nsVec3(-1.0f), nsVec3(1.0f)), new nsDefaultValueAttribute(nsVec3(0.0f))),
    NS_ACCESSOR_PROPERTY("PositiveFalloff", GetPositiveFalloff, SetPositiveFalloff)->AddAttributes(new nsClampValueAttribute(nsVec3(0.0f), nsVec3(1.0f)), new nsDefaultValueAttribute(nsVec3(0.1f, 0.1f, 0.0f))),
    NS_ACCESSOR_PROPERTY("NegativeFalloff", GetNegativeFalloff, SetNegativeFalloff)->AddAttributes(new nsClampValueAttribute(nsVec3(0.0f), nsVec3(1.0f)), new nsDefaultValueAttribute(nsVec3(0.1f, 0.1f, 0.0f))),
    NS_ACCESSOR_PROPERTY("BoxProjection", GetBoxProjection, SetBoxProjection)->AddAttributes(new nsDefaultValueAttribute(true)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_FUNCTIONS
  {
    NS_FUNCTION_PROPERTY(OnObjectCreated),
  }
  NS_END_FUNCTIONS;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgUpdateLocalBounds, OnUpdateLocalBounds),
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
    NS_MESSAGE_HANDLER(nsMsgTransformChanged, OnTransformChanged),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering/Lighting"),
    new nsBoxVisualizerAttribute("Extents", 1.0f, nsColorScheme::LightUI(nsColorScheme::Blue)),
    new nsBoxManipulatorAttribute("Extents", 1.0f, true),
    new nsBoxReflectionProbeVisualizerAttribute("Extents", "InfluenceScale", "InfluenceShift"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsBoxReflectionProbeVisualizerAttribute, 1, nsRTTIDefaultAllocator<nsBoxReflectionProbeVisualizerAttribute>)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_CONSTRUCTOR_PROPERTY(const char*, const char*, const char*),
  }
  NS_END_FUNCTIONS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsBoxReflectionProbeComponentManager::nsBoxReflectionProbeComponentManager(nsWorld* pWorld)
  : nsComponentManager<nsBoxReflectionProbeComponent, nsBlockStorageType::Compact>(pWorld)
{
}

//////////////////////////////////////////////////////////////////////////

nsBoxReflectionProbeComponent::nsBoxReflectionProbeComponent() = default;
nsBoxReflectionProbeComponent::~nsBoxReflectionProbeComponent() = default;

void nsBoxReflectionProbeComponent::SetExtents(const nsVec3& vExtents)
{
  m_vExtents = vExtents;
}

const nsVec3& nsBoxReflectionProbeComponent::GetInfluenceScale() const
{
  return m_vInfluenceScale;
}

void nsBoxReflectionProbeComponent::SetInfluenceScale(const nsVec3& vInfluenceScale)
{
  m_vInfluenceScale = vInfluenceScale;
}

const nsVec3& nsBoxReflectionProbeComponent::GetInfluenceShift() const
{
  return m_vInfluenceShift;
}

void nsBoxReflectionProbeComponent::SetInfluenceShift(const nsVec3& vInfluenceShift)
{
  m_vInfluenceShift = vInfluenceShift;
}

void nsBoxReflectionProbeComponent::SetPositiveFalloff(const nsVec3& vFalloff)
{
  // Does not affect cube generation so m_bStatesDirty is not set.
  m_vPositiveFalloff = vFalloff.CompClamp(nsVec3(nsMath::DefaultEpsilon<float>()), nsVec3(1.0f));
}

void nsBoxReflectionProbeComponent::SetNegativeFalloff(const nsVec3& vFalloff)
{
  // Does not affect cube generation so m_bStatesDirty is not set.
  m_vNegativeFalloff = vFalloff.CompClamp(nsVec3(nsMath::DefaultEpsilon<float>()), nsVec3(1.0f));
}

void nsBoxReflectionProbeComponent::SetBoxProjection(bool bBoxProjection)
{
  m_bBoxProjection = bBoxProjection;
}

const nsVec3& nsBoxReflectionProbeComponent::GetExtents() const
{
  return m_vExtents;
}

void nsBoxReflectionProbeComponent::OnActivated()
{
  GetOwner()->EnableStaticTransformChangesNotifications();
  m_Id = nsReflectionPool::RegisterReflectionProbe(GetWorld(), m_Desc, this);
  GetOwner()->UpdateLocalBounds();
}

void nsBoxReflectionProbeComponent::OnDeactivated()
{
  nsReflectionPool::DeregisterReflectionProbe(GetWorld(), m_Id);
  m_Id.Invalidate();

  GetOwner()->UpdateLocalBounds();
}

void nsBoxReflectionProbeComponent::OnObjectCreated(const nsAbstractObjectNode& node)
{
  m_Desc.m_uniqueID = node.GetGuid();
}

void nsBoxReflectionProbeComponent::OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg)
{
  msg.SetAlwaysVisible(nsDefaultSpatialDataCategories::RenderDynamic);
}

void nsBoxReflectionProbeComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't trigger reflection rendering in shadow or other reflection views.
  if (msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Shadow || msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Reflection)
    return;

  if (m_bStatesDirty)
  {
    m_bStatesDirty = false;
    nsReflectionPool::UpdateReflectionProbe(GetWorld(), m_Id, m_Desc, this);
  }

  auto pRenderData = nsCreateRenderDataForThisFrame<nsReflectionProbeRenderData>(GetOwner());
  pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
  pRenderData->m_vProbePosition = pRenderData->m_GlobalTransform * m_Desc.m_vCaptureOffset;
  pRenderData->m_vHalfExtents = m_vExtents / 2.0f;
  pRenderData->m_vInfluenceScale = m_vInfluenceScale;
  pRenderData->m_vInfluenceShift = m_vInfluenceShift;
  pRenderData->m_vPositiveFalloff = m_vPositiveFalloff;
  pRenderData->m_vNegativeFalloff = m_vNegativeFalloff;
  pRenderData->m_Id = m_Id;
  pRenderData->m_uiIndex = 0;
  if (m_bBoxProjection)
    pRenderData->m_uiIndex |= REFLECTION_PROBE_IS_PROJECTED;

  const nsVec3 vScale = pRenderData->m_GlobalTransform.m_vScale.CompMul(m_vExtents);
  const float fVolume = nsMath::Abs(vScale.x * vScale.y * vScale.z);

  float fPriority = ComputePriority(msg, pRenderData, fVolume, vScale);
  nsReflectionPool::ExtractReflectionProbe(this, msg, pRenderData, GetWorld(), m_Id, fPriority);
}

void nsBoxReflectionProbeComponent::OnTransformChanged(nsMsgTransformChanged& msg)
{
  m_bStatesDirty = true;
}

void nsBoxReflectionProbeComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  s << m_vExtents;
  s << m_vInfluenceScale;
  s << m_vInfluenceShift;
  s << m_vPositiveFalloff;
  s << m_vNegativeFalloff;
  s << m_bBoxProjection;
}

void nsBoxReflectionProbeComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  s >> m_vExtents;
  s >> m_vInfluenceScale;
  s >> m_vInfluenceShift;
  s >> m_vPositiveFalloff;
  s >> m_vNegativeFalloff;
  if (uiVersion >= 2)
  {
    s >> m_bBoxProjection;
  }
}

//////////////////////////////////////////////////////////////////////////

nsBoxReflectionProbeVisualizerAttribute::nsBoxReflectionProbeVisualizerAttribute()
  : nsVisualizerAttribute(nullptr)
{
}

nsBoxReflectionProbeVisualizerAttribute::nsBoxReflectionProbeVisualizerAttribute(const char* szExtentsProperty, const char* szInfluenceScaleProperty, const char* szInfluenceShiftProperty)
  : nsVisualizerAttribute(szExtentsProperty, szInfluenceScaleProperty, szInfluenceShiftProperty)
{
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_BoxReflectionProbeComponent);
