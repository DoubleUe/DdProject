#include "ZombieChaseTestCharacter.h"

#include "AI/ZombieChaseAIController.h"

AZombieChaseTestCharacter::AZombieChaseTestCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass = AZombieChaseAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
