#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DdBaseCharacter.generated.h"

class ADdWeaponActor;
class UCharacterTrajectoryComponent;
class UDdCombatComponent;
class FLifetimeProperty;
struct FDdWeaponTableRow;

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
	virtual void SetAttackBlocked(bool bBlocked);

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttackBlocked() const { return bAttackBlocked; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMovementInputBlocked() const { return bMovementInputBlocked; }

	UFUNCTION(BlueprintPure, Category = "Combat")
	ADdWeaponActor* GetEquippedWeaponActor() const { return EquippedWeaponActor; }

	// 모든 클라이언트에서 몽타주 재생
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayMontage(UAnimMontage* Montage);

protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	virtual void OnConstruction(const FTransform& Transform) override;

	void InitializeWeaponActor();
	void DestroyWeaponActor();
	void AttachWeaponActorToCharacter(const FDdWeaponTableRow& WeaponRow, ADdWeaponActor* WeaponActor) const;

	void ApplyCameraCollisionIgnores();
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
	bool bAttackBlocked = false;

	UPROPERTY(Transient)
	bool bMovementInputBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DDP", meta = (AllowPrivateAccess = "true"))
	int32 WeaponId = 0;

	UPROPERTY(Transient)
	TObjectPtr<ADdWeaponActor> EquippedWeaponActor;
};
