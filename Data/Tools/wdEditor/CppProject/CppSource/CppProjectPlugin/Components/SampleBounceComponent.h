#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <CppProjectPlugin/CppProjectPluginDLL.h>

using SampleBounceComponentManager = wdComponentManagerSimple<class SampleBounceComponent, wdComponentUpdateType::WhenSimulating>;

class SampleBounceComponent : public wdComponent
{
  WD_DECLARE_COMPONENT_TYPE(SampleBounceComponent, wdComponent, SampleBounceComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // wdComponent

public:
  virtual void SerializeComponent(wdWorldWriter& stream) const override;
  virtual void DeserializeComponent(wdWorldReader& stream) override;

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
  wdAngle m_Speed = wdAngle::Degree(90); // [ property ]
};
