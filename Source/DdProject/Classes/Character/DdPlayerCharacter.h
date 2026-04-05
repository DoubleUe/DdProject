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
	void ApplyCameraZoomInput(const FInputActionValue& Value);
	void ApplyMoveInput(const FInputActionValue& Value);
	void ApplyLookInput(const FInputActionValue& Value);
	void TryAttack();

protected:
	virtual void Tick(float DeltaSeconds) override;

private:
	void FinishAttackAnimation();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDdPlayerCameraComponent> PlayerCameraComp;

	UPROPERTY(EditDefaultsOnly, Category = "Animation", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAnimSequenceBase> AttackAnimation;

	FTimerHandle AttackAnimationTimerHandle;
};
