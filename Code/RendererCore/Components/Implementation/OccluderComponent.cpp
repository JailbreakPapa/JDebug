#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/TransformChangedMessage.h>
#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Components/OccluderComponent.h>
#include <RendererCore/Pipeline/RenderData.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsOccluderComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Extents", GetExtents, SetExtents)->AddAttributes(new nsClampValueAttribute(nsVec3(0.0f), {}), new nsDefaultValueAttribute(nsVec3(1.0f))),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgUpdateLocalBounds, OnUpdateLocalBounds),
    NS_MESSAGE_HANDLER(nsMsgExtractOccluderData, OnMsgExtractOccluderData),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
    new nsBoxVisualizerAttribute("Extents", 1.0f, nsColorScheme::LightUI(nsColorScheme::Blue)),
    new nsBoxManipulatorAttribute("Extents", 1.0f, true),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsOccluderComponentManager::nsOccluderComponentManager(nsWorld* pWorld)
  : nsComponentManager<nsOccluderComponent, nsBlockStorageType::FreeList>(pWorld)
{
}

//////////////////////////////////////////////////////////////////////////

nsOccluderComponent::nsOccluderComponent() = default;
nsOccluderComponent::~nsOccluderComponent() = default;

void nsOccluderComponent::SetExtents(const nsVec3& vExtents)
{
  m_vExtents = vExtents;
  m_pOccluderObject.Clear();

  if (IsActiveAndInitialized())
  {
    GetOwner()->UpdateLocalBounds();
  }
}

void nsOccluderComponent::OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg)
{
  if (GetOwner()->IsStatic())
    msg.AddBounds(nsBoundingBoxSphere::MakeFromBox(nsBoundingBox::MakeFromMinMax(-m_vExtents * 0.5f, m_vExtents * 0.5f)), nsDefaultSpatialDataCategories::OcclusionStatic);
  else
    msg.AddBounds(nsBoundingBoxSphere::MakeFromBox(nsBoundingBox::MakeFromMinMax(-m_vExtents * 0.5f, m_vExtents * 0.5f)), nsDefaultSpatialDataCategories::OcclusionDynamic);
}

void nsOccluderComponent::OnMsgExtractOccluderData(nsMsgExtractOccluderData& msg) const
{
  if (IsActiveAndInitialized())
  {
    if (m_pOccluderObject == nullptr)
    {
      m_pOccluderObject = nsRasterizerObject::CreateBox(m_vExtents);
    }

    msg.AddOccluder(m_pOccluderObject.Borrow(), GetOwner()->GetGlobalTransform());
  }
}

void nsOccluderComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  s << m_vExtents;
}

void nsOccluderComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  s >> m_vExtents;
}

void nsOccluderComponent::OnActivated()
{
  m_pOccluderObject.Clear();
  GetOwner()->UpdateLocalBounds();
}

void nsOccluderComponent::OnDeactivated()
{
  m_pOccluderObject.Clear();
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_OccluderComponent);
