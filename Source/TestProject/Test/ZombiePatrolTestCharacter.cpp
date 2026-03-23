#include "ZombiePatrolTestCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Math/UnrealMathUtility.h"

AZombiePatrolTestCharacter::AZombiePatrolTestCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCharacterMovement()->bRunPhysicsWithNoController = true;
}

void AZombiePatrolTestCharacter::BeginPlay()
{
	Super::BeginPlay();

	PatrolStartLocation = GetActorLocation();
	ForwardPatrolLocation = PatrolStartLocation + (GetActorForwardVector() * PatrolDistance);
	bMoveToForwardPoint = true;
}

void AZombiePatrolTestCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdatePatrol();
}

void AZombiePatrolTestCharacter::UpdatePatrol()
{
	const FVector CurrentTarget = bMoveToForwardPoint ? ForwardPatrolLocation : PatrolStartLocation;
	FVector ToTarget = CurrentTarget - GetActorLocation();
	ToTarget.Z = 0.0f;

	if (ToTarget.SizeSquared() <= FMath::Square(ArrivalTolerance))
	{
		bMoveToForwardPoint = !bMoveToForwardPoint;

		const FVector NextTarget = bMoveToForwardPoint ? ForwardPatrolLocation : PatrolStartLocation;
		ToTarget = NextTarget - GetActorLocation();
		ToTarget.Z = 0.0f;
	}

	if (!ToTarget.IsNearlyZero())
	{
		GetCharacterMovement()->AddInputVector(ToTarget.GetSafeNormal() * MovementInputScale);
	}
}
