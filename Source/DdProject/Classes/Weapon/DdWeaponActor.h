#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DdWeaponActor.generated.h"

class USceneComponent;

UCLASS()
class DDPROJECT_API ADdWeaponActor : public AActor
{
	GENERATED_BODY()

public:
	ADdWeaponActor();

	USceneComponent* GetWeaponAttachComponent() const;
	void PrepareForCharacterAttachment(AActor* InOwnerActor);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void OnRep_Owner() override;

private:
	void ApplyCameraCollisionIgnores();
	void ApplyAttachmentCollisionSettings(AActor* InOwnerActor);
};
