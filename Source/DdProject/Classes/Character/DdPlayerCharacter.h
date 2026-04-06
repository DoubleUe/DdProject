#pragma once

#include "CoreMinimal.h"
#include "DdBaseCharacter.h"
#include "DdPlayerCharacter.generated.h"

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
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DDP", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDdPlayerCameraComponent> PlayerCameraComp;
};
