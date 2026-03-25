#pragma once

#include "CoreMinimal.h"
#include "DdBaseCharacter.h"
#include "TimerManager.h"
#include "DdPlayerCharacter.generated.h"

class UClass;
class UAnimSequenceBase;
class UInputAction;
class UDdPlayerCameraComponent;
struct FInputActionValue;

UCLASS()
class DDPROJECT_API ADdPlayerCharacter : public ADdBaseCharacter
{
	GENERATED_BODY()

public:
	ADdPlayerCharacter();
	virtual void SetAttackMovementInputBlocked(bool bBlocked) override;
	virtual void SetAttackInputBlocked(bool bBlocked) override;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void ZoomCamera(float Value);
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Attack();
	void RestoreAnimationBlueprint();
	void TryBlendToMovementAnimation();
	void FinishAttackAnimation();
	bool CanProcessMovementInput() const;
	bool CanProcessAttackInput() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDdPlayerCameraComponent> PlayerCameraComp;

	UPROPERTY(EditDefaultsOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category="Input", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, Category="Animation", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UAnimSequenceBase> AttackAnimation;

	UPROPERTY()
	TObjectPtr<UClass> CharacterAnimBlueprintClass;

	UPROPERTY()
	bool bAttackAnimationPlaying = false;

	UPROPERTY()
	bool bAttackMovementInputBlocked = true;

	UPROPERTY()
	bool bCanTransitionFromAttackToMovement = false;

	// 공격 입력 차단 플래그 (노티파이에 의해 제어)
	UPROPERTY()
	bool bAttackInputBlocked = true;

	FTimerHandle AttackAnimationTimerHandle;
};
