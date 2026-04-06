#pragma once

#include "CoreMinimal.h"
#include "DdBaseCharacter.h"
#include "DdMonsterCharacter.generated.h"

UCLASS()
class DDPROJECT_API ADdMonsterCharacter : public ADdBaseCharacter
{
	GENERATED_BODY()

public:
	ADdMonsterCharacter();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool PlayAttackAnimation();

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTargetActor(AActor* InTarget) { TargetActor = InTarget; }

	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetTargetActor() const { return TargetActor.Get(); }

	virtual void SetMovementInputBlocked(bool bBlocked) override;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMovementBlocked() const { return IsMovementInputBlocked(); }

	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetDistanceToTarget(float InDistance) { DistanceToTarget = InDistance; }

	UFUNCTION(BlueprintPure, Category = "AI")
	float GetDistanceToTarget() const { return DistanceToTarget; }

private:
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	float DistanceToTarget = 0.0f;
};
