#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/BakedProbes/BakedProbesVolumeComponent.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsBakedProbesVolumeComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Extents", GetExtents, SetExtents)->AddAttributes(new nsDefaultValueAttribute(nsVec3(10.0f)), new nsClampValueAttribute(nsVec3(0), nsVariant())),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgUpdateLocalBounds, OnUpdateLocalBounds),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsInDevelopmentAttribute(nsInDevelopmentAttribute::Phase::Beta),
    new nsCategoryAttribute("Lighting/Baking"),
    new nsBoxManipulatorAttribute("Extents", 1.0f, true),
    new nsBoxVisualizerAttribute("Extents", 1.0f, nsColor::OrangeRed),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsBakedProbesVolumeComponent::nsBakedProbesVolumeComponent() = default;
nsBakedProbesVolumeComponent::~nsBakedProbesVolumeComponent() = default;

void nsBakedProbesVolumeComponent::OnActivated()
{
  GetOwner()->UpdateLocalBounds();
}

void nsBakedProbesVolumeComponent::OnDeactivated()
{
  GetOwner()->UpdateLocalBounds();
}

void nsBakedProbesVolumeComponent::SetExtents(const nsVec3& vExtents)
{
  if (m_vExtents != vExtents)
  {
    m_vExtents = vExtents;

    if (IsActiveAndInitialized())
    {
      GetOwner()->UpdateLocalBounds();
    }
  }
}

void nsBakedProbesVolumeComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  s << m_vExtents;
}

void nsBakedProbesVolumeComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  s >> m_vExtents;
}

void nsBakedProbesVolumeComponent::OnUpdateLocalBounds(nsMsgUpdateLocalBounds& ref_msg) const
{
  ref_msg.AddBounds(nsBoundingBoxSphere::MakeFromBox(nsBoundingBox::MakeFromMinMax(-m_vExtents * 0.5f, m_vExtents * 0.5f)), nsInvalidSpatialDataCategory);
}


NS_STATICLINK_FILE(RendererCore, RendererCore_BakedProbes_Implementation_BakedProbesVolumeComponent);
