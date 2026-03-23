#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimationAsset.h"
#include "GameFramework/Character.h"
#include "MonsterCharacter.generated.h"

UCLASS()
class DDPROJECT_API AMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMonsterCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void UpdateMovementAnimation();

	UPROPERTY()
	TObjectPtr<UAnimationAsset> IdleAnimation;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> RunAnimation;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> CurrentLoopAnimation;
};
