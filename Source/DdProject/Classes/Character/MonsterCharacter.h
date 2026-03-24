#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimationAsset.h"
#include "Animation/BlendSpace1D.h"
#include "GameFramework/Character.h"
#include "MonsterCharacter.generated.h"

UCLASS()
class DDPROJECT_API AMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMonsterCharacter();

	// 공격 애니메이션 재생 (AI 컨트롤러에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void PlayAttackAnimation();

	// 공격 애니메이션 재생 중 여부
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return bIsAttacking; }

	// 타겟 액터 설정/가져오기
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTargetActor(AActor* InTarget) { TargetActor = InTarget; }

	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetTargetActor() const { return TargetActor.Get(); }

	// 타겟까지 거리 설정/가져오기
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetDistanceToTarget(float InDistance) { DistanceToTarget = InDistance; }

	UFUNCTION(BlueprintPure, Category = "AI")
	float GetDistanceToTarget() const { return DistanceToTarget; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// 공격 애니메이션
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<UAnimationAsset> AttackAnimation;

private:
	void LoadAnimationAssets();
	void UpdateMovementAnimation(float DeltaSeconds);

	// 공격 애니메이션 종료 콜백
	void OnAttackAnimationEnded();

	UPROPERTY()
	TObjectPtr<UBlendSpace1D> IdleRunBlendSpace;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> IdleAnimation;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> RunAnimation;

	UPROPERTY()
	TObjectPtr<UAnimationAsset> CurrentLoopAnimation;

	FVector PreviousAnimationLocation;
	bool bHasPreviousAnimationLocation;

	// 공격 중 플래그
	bool bIsAttacking = false;

	// AI 타겟 액터
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	// 타겟까지 거리
	float DistanceToTarget = 0.0f;
};
