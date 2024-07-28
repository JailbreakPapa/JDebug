#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Camera.h>
#include <Core/Messages/SetColorMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Lights/LightComponent.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLightRenderData, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsLightRenderData::FillBatchIdAndSortingKey(float fScreenSpaceSize)
{
  m_uiSortingKey = (m_uiShadowDataOffset != nsInvalidIndex) ? 0 : 1;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_ABSTRACT_COMPONENT_TYPE(nsLightComponent, 5)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("UseColorTemperature", GetUsingColorTemperature, SetUsingColorTemperature),
    NS_ACCESSOR_PROPERTY("LightColor", GetLightColor, SetLightColor),
    NS_ACCESSOR_PROPERTY("Temperature", GetTemperature, SetTemperature)->AddAttributes(new nsImageSliderUiAttribute("LightTemperature"), new nsDefaultValueAttribute(6550), new nsClampValueAttribute(1000, 15000)),
    NS_ACCESSOR_PROPERTY("Intensity", GetIntensity, SetIntensity)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(10.0f)),
    NS_ACCESSOR_PROPERTY("SpecularMultiplier", GetSpecularMultiplier, SetSpecularMultiplier)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f)),
    NS_ACCESSOR_PROPERTY("CastShadows", GetCastShadows, SetCastShadows),
    NS_ACCESSOR_PROPERTY("PenumbraSize", GetPenumbraSize, SetPenumbraSize)->AddAttributes(new nsClampValueAttribute(0.0f, 0.5f), new nsDefaultValueAttribute(0.1f), new nsSuffixAttribute(" m")),
    NS_ACCESSOR_PROPERTY("SlopeBias", GetSlopeBias, SetSlopeBias)->AddAttributes(new nsClampValueAttribute(0.0f, 10.0f), new nsDefaultValueAttribute(0.25f)),
    NS_ACCESSOR_PROPERTY("ConstantBias", GetConstantBias, SetConstantBias)->AddAttributes(new nsClampValueAttribute(0.0f, 10.0f), new nsDefaultValueAttribute(0.1f))
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Lighting"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgSetColor, OnMsgSetColor),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_ABSTRACT_COMPONENT_TYPE
// clang-format on

nsLightComponent::nsLightComponent() = default;
nsLightComponent::~nsLightComponent() = default;

void nsLightComponent::SetUsingColorTemperature(bool bUseColorTemperature)
{
  m_bUseColorTemperature = bUseColorTemperature;

  InvalidateCachedRenderData();
}

bool nsLightComponent::GetUsingColorTemperature() const
{
  return m_bUseColorTemperature;
}

void nsLightComponent::SetLightColor(nsColorGammaUB lightColor)
{
  m_LightColor = lightColor;

  InvalidateCachedRenderData();
}

nsColorGammaUB nsLightComponent::GetBaseLightColor() const
{
  return m_LightColor;
}

nsColorGammaUB nsLightComponent::GetLightColor() const
{
  if (m_bUseColorTemperature)
  {
    return nsColor::MakeFromKelvin(m_uiTemperature);
  }
  else
  {
    return m_LightColor;
  }
}

void nsLightComponent::SetIntensity(float fIntensity)
{
  m_fIntensity = nsMath::Max(fIntensity, 0.0f);

  TriggerLocalBoundsUpdate();
}

void nsLightComponent::SetTemperature(nsUInt32 uiTemperature)
{
  m_uiTemperature = nsMath::Clamp(uiTemperature, 1500u, 40000u);

  InvalidateCachedRenderData();
}

nsUInt32 nsLightComponent::GetTemperature() const
{
  return m_uiTemperature;
}

float nsLightComponent::GetIntensity() const
{
  return m_fIntensity;
}

void nsLightComponent::SetSpecularMultiplier(float fSpecularMultiplier)
{
  m_fSpecularMultiplier = nsMath::Max(fSpecularMultiplier, 0.0f);

  InvalidateCachedRenderData();
}

float nsLightComponent::GetSpecularMultiplier() const
{
  return m_fSpecularMultiplier;
}

void nsLightComponent::SetCastShadows(bool bCastShadows)
{
  m_bCastShadows = bCastShadows;

  InvalidateCachedRenderData();
}

bool nsLightComponent::GetCastShadows() const
{
  return m_bCastShadows;
}

void nsLightComponent::SetPenumbraSize(float fPenumbraSize)
{
  m_fPenumbraSize = fPenumbraSize;

  InvalidateCachedRenderData();
}

float nsLightComponent::GetPenumbraSize() const
{
  return m_fPenumbraSize;
}

void nsLightComponent::SetSlopeBias(float fBias)
{
  m_fSlopeBias = fBias;

  InvalidateCachedRenderData();
}

float nsLightComponent::GetSlopeBias() const
{
  return m_fSlopeBias;
}

void nsLightComponent::SetConstantBias(float fBias)
{
  m_fConstantBias = fBias;

  InvalidateCachedRenderData();
}

float nsLightComponent::GetConstantBias() const
{
  return m_fConstantBias;
}

void nsLightComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  s << m_LightColor;
  s << m_fIntensity;
  s << m_fPenumbraSize;
  s << m_fSlopeBias;
  s << m_fConstantBias;
  s << m_bCastShadows;
  s << m_bUseColorTemperature;
  s << m_uiTemperature;
  s << m_fSpecularMultiplier;
}

void nsLightComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  nsStreamReader& s = inout_stream.GetStream();

  s >> m_LightColor;
  s >> m_fIntensity;

  if (uiVersion >= 3)
  {
    s >> m_fPenumbraSize;
  }

  if (uiVersion >= 4)
  {
    s >> m_fSlopeBias;
    s >> m_fConstantBias;
  }

  s >> m_bCastShadows;

  if (uiVersion >= 5)
  {

    s >> m_bUseColorTemperature;
    s >> m_uiTemperature;
    s >> m_fSpecularMultiplier;
  }
}

void nsLightComponent::OnMsgSetColor(nsMsgSetColor& ref_msg)
{
  ref_msg.ModifyColor(m_LightColor);

  InvalidateCachedRenderData();
}

// static
float nsLightComponent::CalculateEffectiveRange(float fRange, float fIntensity)
{
  const float fThreshold = 0.10f; // aggressive threshold to prevent large lights
  const float fEffectiveRange = nsMath::Sqrt(nsMath::Max(0.0f, fIntensity)) / nsMath::Sqrt(fThreshold);

  NS_ASSERT_DEBUG(!nsMath::IsNaN(fEffectiveRange), "Light range is NaN");

  if (fRange <= 0.0f)
  {
    return fEffectiveRange;
  }

  return nsMath::Min(fRange, fEffectiveRange);
}

// static
float nsLightComponent::CalculateScreenSpaceSize(const nsBoundingSphere& sphere, const nsCamera& camera)
{
  if (camera.IsPerspective())
  {
    float dist = (sphere.m_vCenter - camera.GetPosition()).GetLength();
    float fHalfHeight = nsMath::Tan(camera.GetFovY(1.0f) * 0.5f) * dist;
    return nsMath::Pow(sphere.m_fRadius / fHalfHeight, 0.8f); // tweak factor to make transitions more linear.
  }
  else
  {
    float fHalfHeight = camera.GetDimensionY(1.0f) * 0.5f;
    return sphere.m_fRadius / fHalfHeight;
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsLightComponentPatch_1_2 : public nsGraphPatch
{
public:
  nsLightComponentPatch_1_2()
    : nsGraphPatch("nsLightComponent", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override { pNode->RenameProperty("Light Color", "LightColor"); }
};

nsLightComponentPatch_1_2 g_nsLightComponentPatch_1_2;



NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_LightComponent);
