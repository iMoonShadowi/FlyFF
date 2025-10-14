#include "Game/FlyFFGameMode.h"
#include "Game/FlyFFPlayerController.h"
#include "Game/FlyFFGameState.h"
#include "Game/FlyFFPlayerState.h"
#include "UObject/ConstructorHelpers.h"

// Include your character header but DO NOT edit the class itself.
#include "Characters/AFlyFFCharacter.h"

AFlyFFGameMode::AFlyFFGameMode()
{
  PlayerControllerClass = AFlyFFPlayerController::StaticClass();
  GameStateClass = AFlyFFGameState::StaticClass();
  PlayerStateClass = AFlyFFPlayerState::StaticClass();

  // Keep using your existing AFlyFFCharacter as the default pawn.
  DefaultPawnClass = AAFlyFFCharacter::StaticClass();
}
