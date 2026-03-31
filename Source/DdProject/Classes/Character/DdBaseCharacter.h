#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DdBaseCharacter.generated.h"

class UCharacterTrajectoryComponent;

UCLASS(Abstract)
class DDPROJECT_API ADdBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADdBaseCharacter();

	UFUNCTION(BlueprintPure, Category = "Trajectory")
	UCharacterTrajectoryComponent* GetTrajectoryComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Movement|Rotation")
	void ToggleRotationMode();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ToggleWalkSpeed();

	virtual void SetAttackMovementInputBlocked(bool bBlocked) PURE_VIRTUAL(ADdBaseCharacter::SetAttackMovementInputBlocked, );
	virtual void SetAttackInputBlocked(bool bBlocked) PURE_VIRTUAL(ADdBaseCharacter::SetAttackInputBlocked, );

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trajectory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterTrajectoryComponent> TrajectoryComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float NormalWalkSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SlowWalkSpeed = 250.0f;

	UPROPERTY(Transient)
	bool bUseSlowWalkSpeed = false;
};
