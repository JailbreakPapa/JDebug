#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Lights/AmbientLightComponent.h>
#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsAmbientLightComponent, 2, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("TopColor", GetTopColor, SetTopColor)->AddAttributes(new nsDefaultValueAttribute(nsColorGammaUB(nsColor(0.2f, 0.2f, 0.3f)))),
    NS_ACCESSOR_PROPERTY("BottomColor", GetBottomColor, SetBottomColor)->AddAttributes(new nsDefaultValueAttribute(nsColorGammaUB(nsColor(0.1f, 0.1f, 0.15f)))),
    NS_ACCESSOR_PROPERTY("Intensity", GetIntensity, SetIntensity)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f))
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgUpdateLocalBounds, OnUpdateLocalBounds),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Lighting"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsAmbientLightComponent::nsAmbientLightComponent() = default;
nsAmbientLightComponent::~nsAmbientLightComponent() = default;

void nsAmbientLightComponent::Deinitialize()
{
  nsRenderWorld::DeleteCachedRenderData(GetOwner()->GetHandle(), GetHandle());

  SUPER::Deinitialize();
}

void nsAmbientLightComponent::OnActivated()
{
  GetOwner()->UpdateLocalBounds();

  UpdateSkyIrradiance();
}

void nsAmbientLightComponent::OnDeactivated()
{
  GetOwner()->UpdateLocalBounds();

  nsReflectionPool::ResetConstantSkyIrradiance(GetWorld());
}

void nsAmbientLightComponent::SetTopColor(nsColorGammaUB color)
{
  m_TopColor = color;

  if (IsActiveAndInitialized())
  {
    UpdateSkyIrradiance();
  }
}

nsColorGammaUB nsAmbientLightComponent::GetTopColor() const
{
  return m_TopColor;
}

void nsAmbientLightComponent::SetBottomColor(nsColorGammaUB color)
{
  m_BottomColor = color;

  if (IsActiveAndInitialized())
  {
    UpdateSkyIrradiance();
  }
}

nsColorGammaUB nsAmbientLightComponent::GetBottomColor() const
{
  return m_BottomColor;
}

void nsAmbientLightComponent::SetIntensity(float fIntensity)
{
  m_fIntensity = fIntensity;

  if (IsActiveAndInitialized())
  {
    UpdateSkyIrradiance();
  }
}

float nsAmbientLightComponent::GetIntensity() const
{
  return m_fIntensity;
}

void nsAmbientLightComponent::OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg)
{
  msg.SetAlwaysVisible(GetOwner()->IsDynamic() ? nsDefaultSpatialDataCategories::RenderDynamic : nsDefaultSpatialDataCategories::RenderStatic);
}

void nsAmbientLightComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  s << m_TopColor;
  s << m_BottomColor;
  s << m_fIntensity;
}

void nsAmbientLightComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  s >> m_TopColor;
  s >> m_BottomColor;
  s >> m_fIntensity;
}

void nsAmbientLightComponent::UpdateSkyIrradiance()
{
  nsColor topColor = nsColor(m_TopColor) * m_fIntensity;
  nsColor bottomColor = nsColor(m_BottomColor) * m_fIntensity;
  nsColor midColor = nsMath::Lerp(bottomColor, topColor, 0.5f);

  nsAmbientCube<nsColor> ambientLightIrradiance;
  ambientLightIrradiance.m_Values[nsAmbientCubeBasis::PosX] = midColor;
  ambientLightIrradiance.m_Values[nsAmbientCubeBasis::NegX] = midColor;
  ambientLightIrradiance.m_Values[nsAmbientCubeBasis::PosY] = midColor;
  ambientLightIrradiance.m_Values[nsAmbientCubeBasis::NegY] = midColor;
  ambientLightIrradiance.m_Values[nsAmbientCubeBasis::PosZ] = topColor;
  ambientLightIrradiance.m_Values[nsAmbientCubeBasis::NegZ] = bottomColor;

  nsReflectionPool::SetConstantSkyIrradiance(GetWorld(), ambientLightIrradiance);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsAmbientLightComponentPatch_1_2 : public nsGraphPatch
{
public:
  nsAmbientLightComponentPatch_1_2()
    : nsGraphPatch("nsAmbientLightComponent", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    pNode->RenameProperty("Top Color", "TopColor");
    pNode->RenameProperty("Bottom Color", "BottomColor");
  }
};

nsAmbientLightComponentPatch_1_2 g_nsAmbientLightComponentPatch_1_2;



NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_AmbientLightComponent);
