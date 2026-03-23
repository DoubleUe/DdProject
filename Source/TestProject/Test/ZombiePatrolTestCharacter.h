#pragma once

#include "CoreMinimal.h"
#include "../Character/MonsterCharacter.h"
#include "ZombiePatrolTestCharacter.generated.h"

UCLASS()
class TESTPROJECT_API AZombiePatrolTestCharacter : public AMonsterCharacter
{
	GENERATED_BODY()

public:
	AZombiePatrolTestCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "Test Patrol")
	float PatrolDistance = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Test Patrol")
	float ArrivalTolerance = 25.0f;

	UPROPERTY(EditAnywhere, Category = "Test Patrol")
	float MovementInputScale = 1.0f;

private:
	void UpdatePatrol();

	FVector PatrolStartLocation = FVector::ZeroVector;
	FVector ForwardPatrolLocation = FVector::ZeroVector;
	bool bMoveToForwardPoint = true;
};
