#include "AI/ZombieChaseAIController.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"

AZombieChaseAIController::AZombieChaseAIController()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.25f;
	bStartAILogicOnPossess = true;
}

void AZombieChaseAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	RefreshChaseTarget();
	RequestChase();
}

void AZombieChaseAIController::OnUnPossess()
{
	StopMovement();
	ClearFocus(EAIFocusPriority::Gameplay);
	TargetCharacter.Reset();

	Super::OnUnPossess();
}

void AZombieChaseAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	RefreshChaseTarget();
	if (!IsValid(GetPawn()) || !TargetCharacter.IsValid())
	{
		StopMovement();
		ClearFocus(EAIFocusPriority::Gameplay);
		return;
	}

	const float DistanceToTargetSquared = FVector::DistSquared2D(GetPawn()->GetActorLocation(), TargetCharacter->GetActorLocation());
	if (DistanceToTargetSquared <= FMath::Square(ChaseAcceptanceRadius))
	{
		StopMovement();
		return;
	}

	if (GetMoveStatus() != EPathFollowingStatus::Moving)
	{
		RequestChase();
	}
}

void AZombieChaseAIController::RefreshChaseTarget()
{
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
	if (TargetCharacter.Get() == PlayerCharacter)
	{
		return;
	}

	TargetCharacter = PlayerCharacter;
	if (TargetCharacter.IsValid())
	{
		SetFocus(TargetCharacter.Get());
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
	}
}

void AZombieChaseAIController::RequestChase()
{
	if (!IsValid(GetPawn()) || !TargetCharacter.IsValid())
	{
		return;
	}

	MoveToActor(TargetCharacter.Get(), ChaseAcceptanceRadius, true, ShouldUsePathfinding(), true, {}, true);
}

bool AZombieChaseAIController::ShouldUsePathfinding() const
{
	if (!bPreferNavigationPathfinding)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	return NavigationSystem != nullptr
		&& NavigationSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) != nullptr;
}
