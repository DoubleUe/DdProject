#pragma once

#include "CoreMinimal.h"
#include "DdBaseCharacter.h"
#include "TimerManager.h"
#include "DdPlayerCharacter.generated.h"

class UAnimSequenceBase;
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
	void ApplyCameraZoomInput(const FInputActionValue& Value);
	void ApplyMoveInput(const FInputActionValue& Value);
	void ApplyLookInput(const FInputActionValue& Value);
	void TryAttack();

protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	void FinishAttackAnimation();
	bool CanProcessAttackInput() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDdPlayerCameraComponent> PlayerCameraComp;

	UPROPERTY(EditDefaultsOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimSequenceBase> AttackAnimation;

	UPROPERTY()
	bool bAttackAnimationPlaying = false;

	UPROPERTY()
	bool bAttackMovementInputBlocked = true;

	// Controlled by animation notify states to gate attack input windows.
	UPROPERTY()
	bool bAttackInputBlocked = true;

	FTimerHandle AttackAnimationTimerHandle;
};
