#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DdBaseAnimInstance.generated.h"

class UCharacterMovementComponent;
class UCharacterTrajectoryComponent;

UCLASS(Blueprintable, BlueprintType)
class DDPROJECT_API UDdBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Character|Components")
	TObjectPtr<UCharacterTrajectoryComponent> TrajectoryComponent = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "Character|Components")
	TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent = nullptr;

private:
	void CacheOwnerComponents();
};
