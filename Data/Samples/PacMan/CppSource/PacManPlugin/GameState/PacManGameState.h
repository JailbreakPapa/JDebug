#pragma once

#include <Core/Input/Declarations.h>
#include <Core/World/Declarations.h>
#include <GameEngine/GameApplication/GameApplication.h>
#include <GameEngine/GameState/FallbackGameState.h>
#include <GameEngine/GameState/GameState.h>
#include <PacManPlugin/PacManPluginDLL.h>

// Every game can have a single 'game state' for high-level logic.
// For more details, see https://wdengine.net/pages/docs/runtime/application/game-state.html
class PacManGameState : public wdFallbackGameState
{
  WD_ADD_DYNAMIC_REFLECTION(PacManGameState, wdFallbackGameState);

public:
  PacManGameState();
  ~PacManGameState();

  // The engine looks at all available game-states and has to use a single one.
  // We need to override this, to tell it that this game-state is important and should be used.
  // Otherwise it would use the next best game-state, which might be the wdFallbackGameState.
  virtual wdGameStatePriority DeterminePriority(wdWorld* pWorld) const override;

  // Called at the start of each frame. The typical per-frame decisions should be done here.
  virtual void ProcessInput() override;

protected:
  virtual void ConfigureInputActions() override;
  virtual void ConfigureMainCamera() override;

private:
  virtual void OnActivation(wdWorld* pWorld, const wdTransform* pStartPosition) override;
  virtual void OnDeactivation() override;
  virtual void AfterWorldUpdate() override;
  virtual wdResult SpawnPlayer(const wdTransform* pStartPosition) override;

  void ResetState();

  // How many coins we have in the scene, in total.
  wdUInt32 m_uiNumCoinsTotal = 0;
};
