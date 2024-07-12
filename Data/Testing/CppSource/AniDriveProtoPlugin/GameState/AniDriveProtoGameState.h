/*
 *   Copyright (c) 2023 WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Core/Input/Declarations.h>
#include <Core/World/Declarations.h>
#include <AniDriveProtoPlugin/AniDriveProtoPluginDLL.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/FallbackGameState.h>
#include <GameEngine/GameState/GameState.h>

class AniDriveProtoGameState : public nsFallbackGameState
{
  NS_ADD_DYNAMIC_REFLECTION(AniDriveProtoGameState, nsFallbackGameState);

public:
  AniDriveProtoGameState();
  ~AniDriveProtoGameState();

  virtual nsGameStatePriority DeterminePriority(nsWorld* pWorld) const override;

  virtual void ProcessInput() override;

protected:
  virtual void ConfigureMainWindowInputDevices(nsWindow* pWindow) override;
  virtual void ConfigureInputActions() override;
  virtual void ConfigureMainCamera() override;

private:
  virtual void OnActivation(nsWorld* pWorld, const nsTransform* pStartPosition) override;
  virtual void OnDeactivation() override;
  virtual void BeforeWorldUpdate() override;
  virtual void AfterWorldUpdate() override;

  nsDeque<nsGameObjectHandle> m_SpawnedObjects;
};
