/*
 *   Copyright (c) 2023 WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <AniDriveProtoPlugin/AniDriveProtoPluginDLL.h>

using SampleBounceComponentManager = nsComponentManagerSimple<class SampleBounceComponent, nsComponentUpdateType::WhenSimulating>;

class SampleBounceComponent : public nsComponent
{
  NS_DECLARE_COMPONENT_TYPE(SampleBounceComponent, nsComponent, SampleBounceComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // nsComponent

public:
  virtual void SerializeComponent(nsWorldWriter& stream) const override;
  virtual void DeserializeComponent(nsWorldReader& stream) override;

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // SampleBounceComponent

public:
  SampleBounceComponent();
  ~SampleBounceComponent();

private:
  void Update();

  float m_fAmplitude = 1.0f;             // [ property ]
  nsAngle m_Speed = nsAngle::MakeFromDegree(90); // [ property ]
};
