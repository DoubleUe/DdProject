#include "GameLevelGameMode.h"

#include "../Character/ThirdPersonSampleCharacter.h"
#include "../PlayerController/GameplayPlayerController.h"

AGameLevelGameMode::AGameLevelGameMode()
{
	PlayerControllerClass = AGameplayPlayerController::StaticClass();
	DefaultPawnClass = AThirdPersonSampleCharacter::StaticClass();
}
