#pragma once

#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>
#include <Core/World/World.h>
#include <PacManPlugin/PacManPluginDLL.h>

using GhostComponentManager = wdComponentManagerSimple<class GhostComponent, wdComponentUpdateType::WhenSimulating>;

class GhostComponent : public wdComponent
{
  WD_DECLARE_COMPONENT_TYPE(GhostComponent, wdComponent, GhostComponentManager);

  //////////////////////////////////////////////////////////////////////////
  // wdComponent

public:
  virtual void SerializeComponent(wdWorldWriter& stream) const override;
  virtual void DeserializeComponent(wdWorldReader& stream) override;

protected:
  virtual void OnSimulationStarted() override;

  //////////////////////////////////////////////////////////////////////////
  // GhostComponent

public:
  GhostComponent();
  ~GhostComponent();

private:
  void Update();

  WalkDirection m_Direction = WalkDirection::Up;

  wdPrefabResourceHandle m_hDisappear;

  wdSharedPtr<wdBlackboard> m_pStateBlackboard;

  float m_fSpeed = 2.0f;
};
