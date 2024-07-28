#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/SphereReflectionProbeComponent.h>

#include <../../Data/Base/Shaders/Common/LightData.h>
#include <Core/Messages/TransformChangedMessage.h>
#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Pipeline/View.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsSphereReflectionProbeComponent, 2, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Radius", GetRadius, SetRadius)->AddAttributes(new nsClampValueAttribute(0.0f, {}), new nsDefaultValueAttribute(5.0f)),
    NS_ACCESSOR_PROPERTY("Falloff", GetFalloff, SetFalloff)->AddAttributes(new nsClampValueAttribute(0.0f, 1.0f), new nsDefaultValueAttribute(0.1f)),
    NS_ACCESSOR_PROPERTY("SphereProjection", GetSphereProjection, SetSphereProjection)->AddAttributes(new nsDefaultValueAttribute(true)),
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
    new nsSphereVisualizerAttribute("Radius", nsColorScheme::LightUI(nsColorScheme::Blue)),
    new nsSphereManipulatorAttribute("Radius"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsSphereReflectionProbeComponentManager::nsSphereReflectionProbeComponentManager(nsWorld* pWorld)
  : nsComponentManager<nsSphereReflectionProbeComponent, nsBlockStorageType::Compact>(pWorld)
{
}

//////////////////////////////////////////////////////////////////////////

nsSphereReflectionProbeComponent::nsSphereReflectionProbeComponent() = default;
nsSphereReflectionProbeComponent::~nsSphereReflectionProbeComponent() = default;

void nsSphereReflectionProbeComponent::SetRadius(float fRadius)
{
  m_fRadius = nsMath::Max(fRadius, 0.0f);
  m_bStatesDirty = true;
}

float nsSphereReflectionProbeComponent::GetRadius() const
{
  return m_fRadius;
}

void nsSphereReflectionProbeComponent::SetFalloff(float fFalloff)
{
  m_fFalloff = nsMath::Clamp(fFalloff, nsMath::DefaultEpsilon<float>(), 1.0f);
}

void nsSphereReflectionProbeComponent::SetSphereProjection(bool bSphereProjection)
{
  m_bSphereProjection = bSphereProjection;
}

void nsSphereReflectionProbeComponent::OnActivated()
{
  GetOwner()->EnableStaticTransformChangesNotifications();
  m_Id = nsReflectionPool::RegisterReflectionProbe(GetWorld(), m_Desc, this);
  GetOwner()->UpdateLocalBounds();
}

void nsSphereReflectionProbeComponent::OnDeactivated()
{
  nsReflectionPool::DeregisterReflectionProbe(GetWorld(), m_Id);
  m_Id.Invalidate();

  GetOwner()->UpdateLocalBounds();
}

void nsSphereReflectionProbeComponent::OnObjectCreated(const nsAbstractObjectNode& node)
{
  m_Desc.m_uniqueID = node.GetGuid();
}

void nsSphereReflectionProbeComponent::OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg)
{
  msg.SetAlwaysVisible(nsDefaultSpatialDataCategories::RenderDynamic);
}

void nsSphereReflectionProbeComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
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
  pRenderData->m_vHalfExtents = nsVec3(m_fRadius);
  pRenderData->m_vInfluenceScale = nsVec3(1.0f);
  pRenderData->m_vInfluenceShift = nsVec3(0.0f);
  pRenderData->m_vPositiveFalloff = nsVec3(m_fFalloff);
  pRenderData->m_vNegativeFalloff = nsVec3(m_fFalloff);
  pRenderData->m_Id = m_Id;
  pRenderData->m_uiIndex = REFLECTION_PROBE_IS_SPHERE;
  if (m_bSphereProjection)
    pRenderData->m_uiIndex |= REFLECTION_PROBE_IS_PROJECTED;

  const nsVec3 vScale = pRenderData->m_GlobalTransform.m_vScale * m_fRadius;
  constexpr float fSphereConstant = (4.0f / 3.0f) * nsMath::Pi<float>();
  const float fEllipsoidVolume = fSphereConstant * nsMath::Abs(vScale.x * vScale.y * vScale.z);

  float fPriority = ComputePriority(msg, pRenderData, fEllipsoidVolume, vScale);
  nsReflectionPool::ExtractReflectionProbe(this, msg, pRenderData, GetWorld(), m_Id, fPriority);
}

void nsSphereReflectionProbeComponent::OnTransformChanged(nsMsgTransformChanged& msg)
{
  m_bStatesDirty = true;
}

void nsSphereReflectionProbeComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  s << m_fRadius;
  s << m_fFalloff;
  s << m_bSphereProjection;
}

void nsSphereReflectionProbeComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  s >> m_fRadius;
  s >> m_fFalloff;
  if (uiVersion >= 2)
  {
    s >> m_bSphereProjection;
  }
  else
  {
    m_bSphereProjection = false;
  }
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/GraphPatch.h>

class nsSphereReflectionProbeComponent_1_2 : public nsGraphPatch
{
public:
  nsSphereReflectionProbeComponent_1_2()
    : nsGraphPatch("nsSphereReflectionProbeComponent", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    pNode->AddProperty("SphereProjection", false);
  }
};

nsSphereReflectionProbeComponent_1_2 g_nsSphereReflectionProbeComponent_1_2;

NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_SphereReflectionProbeComponent);
