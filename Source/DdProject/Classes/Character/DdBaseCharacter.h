#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DdBaseCharacter.generated.h"

class UCharacterTrajectoryComponent;
class UDdCombatComponent;
class FLifetimeProperty;

UCLASS(Abstract)
class DDPROJECT_API ADdBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ADdBaseCharacter();

	UFUNCTION(BlueprintPure, Category = "Trajectory")
	UCharacterTrajectoryComponent* GetTrajectoryComponent() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	UDdCombatComponent* GetCombatComponent() const { return CombatComponent; }

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
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void ApplyCameraCollisionIgnores();
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DDP", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterTrajectoryComponent> TrajectoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DDP", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDdCombatComponent> CombatComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DDP", meta = (AllowPrivateAccess = "true"))
	float NormalWalkSpeed = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DDP", meta = (AllowPrivateAccess = "true"))
	float SlowWalkSpeed = 250.0f;

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
