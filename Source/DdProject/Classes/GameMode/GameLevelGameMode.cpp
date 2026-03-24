#include "GameLevelGameMode.h"

#include "../PlayerController/GameplayPlayerController.h"
#include "UObject/ConstructorHelpers.h"

AGameLevelGameMode::AGameLevelGameMode()
{
	PlayerControllerClass = AGameplayPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PlayerBPClass(TEXT("/Game/Characters/Player/BP_PlayerCharacter"));
	if (PlayerBPClass.Succeeded())
	{
		DefaultPawnClass = PlayerBPClass.Class;
	}
}
