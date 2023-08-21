#pragma once

#include <Core/Input/Declarations.h>
#include <Core/World/Declarations.h>
#include <CppProjectPlugin/CppProjectPluginDLL.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/FallbackGameState.h>
#include <GameEngine/GameState/GameState.h>

class CppProjectGameState : public wdFallbackGameState
{
  WD_ADD_DYNAMIC_REFLECTION(CppProjectGameState, wdFallbackGameState);

public:
  CppProjectGameState();
  ~CppProjectGameState();

  virtual wdGameStatePriority DeterminePriority(wdWorld* pWorld) const override;

  virtual void ProcessInput() override;

protected:
  virtual void ConfigureMainWindowInputDevices(wdWindow* pWindow) override;
  virtual void ConfigureInputActions() override;
  virtual void ConfigureMainCamera() override;

private:
  virtual void OnActivation(wdWorld* pWorld, const wdTransform* pStartPosition) override;
  virtual void OnDeactivation() override;
  virtual void BeforeWorldUpdate() override;
  virtual void AfterWorldUpdate() override;

  wdDeque<wdGameObjectHandle> m_SpawnedObjects;
};
