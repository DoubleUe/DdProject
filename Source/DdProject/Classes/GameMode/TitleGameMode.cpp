#include "TitleGameMode.h"

#include "../PlayerController/TitlePlayerController.h"

ATitleGameMode::ATitleGameMode()
{
	PlayerControllerClass = ATitlePlayerController::StaticClass();
}
