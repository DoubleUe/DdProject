#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DdBaseCharacter.generated.h"

class UCharacterTrajectoryComponent;
class UStaticMeshComponent;

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
	virtual void OnConstruction(const FTransform& Transform) override;

	void InitializeEquippedStaticMeshAttachment();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trajectory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterTrajectoryComponent> TrajectoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> EquippedStaticMeshComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float NormalWalkSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float SlowWalkSpeed = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	FName EquippedStaticMeshParentSocketName = TEXT("hand_r_weapon");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipment", meta = (AllowPrivateAccess = "true"))
	FName EquippedStaticMeshChildSocketName = TEXT("hand");

	UPROPERTY(Transient)
	bool bUseSlowWalkSpeed = false;
};
