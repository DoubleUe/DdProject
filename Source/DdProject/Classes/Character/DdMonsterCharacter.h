#pragma once

#include "CoreMinimal.h"
#include "DdBaseCharacter.h"
#include "DdMonsterCharacter.generated.h"

class UAnimationAsset;
class UBlendSpace1D;
class USkeletalMeshComponent;

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

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UAnimationAsset> AttackAnimation;

private:
	void LoadAnimationAssets();
	void UpdateMovementAnimation(float DeltaSeconds);
	void OnAttackAnimationEnded();
	USkeletalMeshComponent* FindSkeletalMeshComponent() const;

	UPROPERTY()
	TObjectPtr<UBlendSpace1D> IdleRunBlendSpace;

	bool bIsPlayingBlendSpace = false;
	FVector PreviousAnimationLocation;
	bool bHasPreviousAnimationLocation = false;

	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	float DistanceToTarget = 0.0f;
};
