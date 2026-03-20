#include "GameLevelGameMode.h"

#include "../Character/PlayerCharacter.h"
#include "../PlayerController/GameplayPlayerController.h"

AGameLevelGameMode::AGameLevelGameMode()
{
	PlayerControllerClass = AGameplayPlayerController::StaticClass();
	DefaultPawnClass = APlayerCharacter::StaticClass();
}
