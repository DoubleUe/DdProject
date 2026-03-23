#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class UClass;
class UAnimSequenceBase;
class UInputAction;
class USpringArmComponent;
struct FInputActionValue;

UCLASS()
class DDPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	void SetAttackMovementInputBlocked(bool bBlocked);

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
	void UpdateCameraZoom(float DeltaSeconds);
	void RefreshCameraZoomBounds();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	float CameraZoomStep = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category="Camera", meta=(AllowPrivateAccess="true", ClampMin="0.1"))
	float CameraZoomInterpSpeed = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	float MinCameraBoomLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	float MaxCameraBoomLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera", meta=(AllowPrivateAccess="true"))
	float DesiredCameraBoomLength = 0.0f;

	UPROPERTY()
	bool bCameraZoomBoundsDirty = true;

	UPROPERTY()
	float CachedCapsuleZoomLimit = -1.0f;

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
	bool bAttackMovementInputBlocked = false;

	UPROPERTY()
	bool bCanTransitionFromAttackToMovement = false;

	FTimerHandle AttackAnimationTimerHandle;
};
