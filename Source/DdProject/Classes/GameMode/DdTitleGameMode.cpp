#include "DdTitleGameMode.h"

#include "../PlayerController/DdTitlePlayerController.h"

ADdTitleGameMode::ADdTitleGameMode()
{
	PlayerControllerClass = ADdTitlePlayerController::StaticClass();
}
