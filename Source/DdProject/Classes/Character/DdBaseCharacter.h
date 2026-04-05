#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DdBaseCharacter.generated.h"

class UCharacterTrajectoryComponent;
class UStaticMeshComponent;
class FLifetimeProperty;

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

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetMovementInputBlocked(bool bBlocked);
	virtual void SetAttackInputBlocked(bool bBlocked);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttackInputBlocked() const { return bAttackInputBlocked; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return bIsAttacking; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanAttack() const { return !bAttackInputBlocked && !bIsAttacking; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMovementInputBlocked() const { return bMovementInputBlocked; }

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

	void InitializeEquippedStaticMeshAttachment();
	void SetAttacking(bool bInIsAttacking) { bIsAttacking = bInIsAttacking; }
	void ApplyRotationModeFromState();
	void SetUseControllerDesiredRotationMode(bool bInUseControllerDesiredRotationMode);
	void ApplyWalkSpeedFromState();
	void SetUseSlowWalkSpeed(bool bInUseSlowWalkSpeed);

	UFUNCTION()
	void OnRep_UseControllerDesiredRotationMode();

	UFUNCTION()
	void OnRep_UseSlowWalkSpeed();

	UFUNCTION(Server, Reliable)
	void ServerSetUseControllerDesiredRotationMode(bool bInUseControllerDesiredRotationMode);

	UFUNCTION(Server, Reliable)
	void ServerSetUseSlowWalkSpeed(bool bInUseSlowWalkSpeed);

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

	UPROPERTY(ReplicatedUsing = OnRep_UseControllerDesiredRotationMode, Transient)
	bool bUseControllerDesiredRotationMode = false;

	UPROPERTY(ReplicatedUsing = OnRep_UseSlowWalkSpeed, Transient)
	bool bUseSlowWalkSpeed = false;

	UPROPERTY(Transient)
	bool bAttackInputBlocked = false;

	UPROPERTY(Transient)
	bool bIsAttacking = false;

	UPROPERTY(Transient)
	bool bMovementInputBlocked = false;
};
