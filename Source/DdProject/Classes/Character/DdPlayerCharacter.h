#pragma once

#include "CoreMinimal.h"
#include "DdBaseCharacter.h"
#include "DdPlayerCharacter.generated.h"

class ADdWeaponActor;
class UDdPlayerCameraComponent;
struct FInputActionValue;
struct FDdWeaponTableRow;

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
	virtual void Destroyed() override;

private:
	void InitializeWeaponActor();
	void DestroyWeaponActor();
	void AttachWeaponActorToCharacter(const FDdWeaponTableRow& WeaponRow, ADdWeaponActor* WeaponActor) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "DDP", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDdPlayerCameraComponent> PlayerCameraComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DDP", meta = (AllowPrivateAccess = "true"))
	int32 WeaponId = 0;

	UPROPERTY(Transient)
	TObjectPtr<ADdWeaponActor> EquippedWeaponActor;
};
