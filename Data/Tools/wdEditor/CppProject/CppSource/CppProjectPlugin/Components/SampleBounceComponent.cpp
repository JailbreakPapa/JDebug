#include <CppProjectPlugin/CppProjectPluginPCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <CppProjectPlugin/Components/SampleBounceComponent.h>

// clang-format off
WD_BEGIN_COMPONENT_TYPE(SampleBounceComponent, 1 /* version */, wdComponentMode::Dynamic)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Amplitude", m_fAmplitude)->AddAttributes(new wdDefaultValueAttribute(1), new wdClampValueAttribute(0, 10)),
    WD_MEMBER_PROPERTY("Speed", m_Speed)->AddAttributes(new wdDefaultValueAttribute(wdAngle::Degree(90))),
  }
  WD_END_PROPERTIES;

  WD_BEGIN_ATTRIBUTES
  {
    new wdCategoryAttribute("CppProject"), // Component menu group
  }
  WD_END_ATTRIBUTES;
}
WD_END_COMPONENT_TYPE
// clang-format on

SampleBounceComponent::SampleBounceComponent() = default;
SampleBounceComponent::~SampleBounceComponent() = default;

void SampleBounceComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  // this component doesn't need to anything for initialization
}

void SampleBounceComponent::Update()
{
  const wdTime curTime = GetWorld()->GetClock().GetAccumulatedTime();
  const wdAngle curAngle = curTime.AsFloatInSeconds() * m_Speed;
  const float curHeight = wdMath::Sin(curAngle) * m_fAmplitude;

  GetOwner()->SetLocalPosition(wdVec3(0, 0, curHeight));
}

void SampleBounceComponent::SerializeComponent(wdWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  if (OWNTYPE::GetStaticRTTI()->GetTypeVersion() == 1)
  {
    // this automatically serializes all properties
    // if you need more control, increase the component 'version' at the top of this file
    // and then use the code path below for manual serialization
    wdReflectionSerializer::WriteObjectToBinary(s, GetDynamicRTTI(), this);
  }
  else
  {
    // do custom serialization, for example:
    // s << m_fAmplitude;
    // s << m_Speed;
  }
}

void SampleBounceComponent::DeserializeComponent(wdWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const wdUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  if (uiVersion == 1)
  {
    // this automatically de-serializes all properties
    // if you need more control, increase the component 'version' at the top of this file
    // and then use the code path below for manual de-serialization
    wdReflectionSerializer::ReadObjectPropertiesFromBinary(s, *GetDynamicRTTI(), this);
  }
  else
  {
    // do custom de-serialization, for example:
    // s >> m_fAmplitude;
    // s >> m_Speed;
  }
}
