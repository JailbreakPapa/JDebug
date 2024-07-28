#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/BakedProbes/BakingInterface.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsBakingSettings, nsNoBase, 1, nsRTTIDefaultAllocator<nsBakingSettings>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ProbeSpacing", m_vProbeSpacing)->AddAttributes(new nsDefaultValueAttribute(nsVec3(4)), new nsClampValueAttribute(nsVec3(0.1f), nsVariant())),
    NS_MEMBER_PROPERTY("NumSamplesPerProbe", m_uiNumSamplesPerProbe)->AddAttributes(new nsDefaultValueAttribute(128), new nsClampValueAttribute(32, 1024)),
    NS_MEMBER_PROPERTY("MaxRayDistance", m_fMaxRayDistance)->AddAttributes(new nsDefaultValueAttribute(1000), new nsClampValueAttribute(1, nsVariant())),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

static nsTypeVersion s_BakingSettingsVersion = 1;
nsResult nsBakingSettings::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream.WriteVersion(s_BakingSettingsVersion);

  inout_stream << m_vProbeSpacing;
  inout_stream << m_uiNumSamplesPerProbe;
  inout_stream << m_fMaxRayDistance;

  return NS_SUCCESS;
}

nsResult nsBakingSettings::Deserialize(nsStreamReader& inout_stream)
{
  const nsTypeVersion version = inout_stream.ReadVersion(s_BakingSettingsVersion);
  NS_IGNORE_UNUSED(version);

  inout_stream >> m_vProbeSpacing;
  inout_stream >> m_uiNumSamplesPerProbe;
  inout_stream >> m_fMaxRayDistance;

  return NS_SUCCESS;
}


NS_STATICLINK_FILE(RendererCore, RendererCore_BakedProbes_Implementation_BakingInterface);
