#include "DdGameLevelGameMode.h"

#include "../PlayerController/DdGameplayPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ADdGameLevelGameMode::ADdGameLevelGameMode()
{
	PlayerControllerClass = ADdGameplayPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PlayerBPClass(TEXT("/Game/Characters/Player/BP_PlayerCharacter"));
	if (PlayerBPClass.Succeeded())
	{
		DefaultPawnClass = PlayerBPClass.Class;
	}
}
