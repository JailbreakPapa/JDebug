#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Components/FogComponent.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsFogRenderData, 1, nsRTTIDefaultAllocator<nsFogRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_COMPONENT_TYPE(nsFogComponent, 2, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Color", GetColor, SetColor)->AddAttributes(new nsDefaultValueAttribute(nsColorGammaUB(nsColor(0.2f, 0.2f, 0.3f)))),
    NS_ACCESSOR_PROPERTY("Density", GetDensity, SetDensity)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f)),
    NS_ACCESSOR_PROPERTY("HeightFalloff", GetHeightFalloff, SetHeightFalloff)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(10.0f)),
    NS_ACCESSOR_PROPERTY("ModulateWithSkyColor", GetModulateWithSkyColor, SetModulateWithSkyColor),
    NS_ACCESSOR_PROPERTY("SkyDistance", GetSkyDistance, SetSkyDistance)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1000.0f)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgUpdateLocalBounds, OnUpdateLocalBounds),
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Effects"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsFogComponent::nsFogComponent() = default;
nsFogComponent::~nsFogComponent() = default;

void nsFogComponent::Deinitialize()
{
  nsRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());

  SUPER::Deinitialize();
}

void nsFogComponent::OnActivated()
{
  GetOwner()->UpdateLocalBounds();
}

void nsFogComponent::OnDeactivated()
{
  GetOwner()->UpdateLocalBounds();
}

void nsFogComponent::SetColor(nsColor color)
{
  m_Color = color;
  SetModified(NS_BIT(1));
}

nsColor nsFogComponent::GetColor() const
{
  return m_Color;
}

void nsFogComponent::SetDensity(float fDensity)
{
  m_fDensity = nsMath::Max(fDensity, 0.0f);
  SetModified(NS_BIT(2));
}

float nsFogComponent::GetDensity() const
{
  return m_fDensity;
}

void nsFogComponent::SetHeightFalloff(float fHeightFalloff)
{
  m_fHeightFalloff = nsMath::Max(fHeightFalloff, 0.0f);
  SetModified(NS_BIT(3));
}

float nsFogComponent::GetHeightFalloff() const
{
  return m_fHeightFalloff;
}

void nsFogComponent::SetModulateWithSkyColor(bool bModulate)
{
  m_bModulateWithSkyColor = bModulate;
  SetModified(NS_BIT(4));
}

bool nsFogComponent::GetModulateWithSkyColor() const
{
  return m_bModulateWithSkyColor;
}

void nsFogComponent::SetSkyDistance(float fDistance)
{
  m_fSkyDistance = fDistance;
  SetModified(NS_BIT(5));
}

float nsFogComponent::GetSkyDistance() const
{
  return m_fSkyDistance;
}

void nsFogComponent::OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg)
{
  msg.SetAlwaysVisible(GetOwner()->IsDynamic() ? nsDefaultSpatialDataCategories::RenderDynamic : nsDefaultSpatialDataCategories::RenderStatic);
}

void nsFogComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  if (msg.m_OverrideCategory != nsInvalidRenderDataCategory)
    return;

  auto pRenderData = nsCreateRenderDataForThisFrame<nsFogRenderData>(GetOwner());

  pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
  pRenderData->m_Color = m_Color;
  pRenderData->m_fDensity = m_fDensity / 100.0f;
  pRenderData->m_fHeightFalloff = m_fHeightFalloff;
  pRenderData->m_fInvSkyDistance = m_bModulateWithSkyColor ? 1.0f / m_fSkyDistance : 0.0f;

  msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::Light, nsRenderData::Caching::IfStatic);
}

void nsFogComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  s << m_Color;
  s << m_fDensity;
  s << m_fHeightFalloff;
  s << m_fSkyDistance;
  s << m_bModulateWithSkyColor;
}

void nsFogComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  s >> m_Color;
  s >> m_fDensity;
  s >> m_fHeightFalloff;

  if (uiVersion >= 2)
  {
    s >> m_fSkyDistance;
    s >> m_bModulateWithSkyColor;
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_FogComponent);
