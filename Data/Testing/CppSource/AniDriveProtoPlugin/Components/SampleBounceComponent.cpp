/*
 *   Copyright (c) 2023 WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <AniDriveProtoPlugin/AniDriveProtoPluginPCH.h>

#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <AniDriveProtoPlugin/Components/SampleBounceComponent.h>

// clang-format off
NS_BEGIN_COMPONENT_TYPE(SampleBounceComponent, 1 /* version */, nsComponentMode::Dynamic)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Amplitude", m_fAmplitude)->AddAttributes(new nsDefaultValueAttribute(1), new nsClampValueAttribute(0, 10)),
    NS_MEMBER_PROPERTY("Speed", m_Speed)->AddAttributes(new nsDefaultValueAttribute(nsAngle::MakeFromDegree(90))),
  }
  NS_END_PROPERTIES;

  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("AniDriveProto"), // Component menu group
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
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
  const nsTime curTime = GetWorld()->GetClock().GetAccumulatedTime();
  const nsAngle curAngle = curTime.AsFloatInSeconds() * m_Speed;
  const float curHeight = nsMath::Sin(curAngle) * m_fAmplitude;

  GetOwner()->SetLocalPosition(nsVec3(0, 0, curHeight));
}

void SampleBounceComponent::SerializeComponent(nsWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  if (OWNTYPE::GetStaticRTTI()->GetTypeVersion() == 1)
  {
    // this automatically serializes all properties
    // if you need more control, increase the component 'version' at the top of this file
    // and then use the code path below for manual serialization
    nsReflectionSerializer::WriteObjectToBinary(s, GetDynamicRTTI(), this);
  }
  else
  {
    // do custom serialization, for example:
    // s << m_fAmplitude;
    // s << m_Speed;
  }
}

void SampleBounceComponent::DeserializeComponent(nsWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  if (uiVersion == 1)
  {
    // this automatically de-serializes all properties
    // if you need more control, increase the component 'version' at the top of this file
    // and then use the code path below for manual de-serialization
    nsReflectionSerializer::ReadObjectPropertiesFromBinary(s, *GetDynamicRTTI(), this);
  }
  else
  {
    // do custom de-serialization, for example:
    // s >> m_fAmplitude;
    // s >> m_Speed;
  }
}
