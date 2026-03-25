#pragma once

#include "CoreMinimal.h"
#include "DdBaseCharacter.h"
#include "DdMonsterCharacter.generated.h"

class UAnimationAsset;
class UBlendSpace1D;

UCLASS()
class DDPROJECT_API ADdMonsterCharacter : public ADdBaseCharacter
{
	GENERATED_BODY()

public:
	ADdMonsterCharacter();

	// 공격 애니메이션 재생 (AI 컨트롤러에서 호출), 성공 여부 반환
	UFUNCTION(BlueprintCallable, Category = "Combat")
	bool PlayAttackAnimation();

	// 공격 애니메이션 재생 중 여부
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return bIsAttacking; }

	// 공격 입력 가능 여부 (노티파이에 의해 제어)
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool CanAttack() const { return !bAttackInputBlocked && !bIsAttacking; }

	// 타겟 액터 설정/가져오기
	UFUNCTION(BlueprintCallable, Category = "AI")
	void SetTargetActor(AActor* InTarget) { TargetActor = InTarget; }

	UFUNCTION(BlueprintPure, Category = "AI")
	AActor* GetTargetActor() const { return TargetActor.Get(); }

	// 공격 중 이동 차단 설정 (노티파이에서 호출)
	virtual void SetAttackMovementInputBlocked(bool bBlocked) override;

	// 공격 입력 차단 설정 (노티파이에서 호출)
	virtual void SetAttackInputBlocked(bool bBlocked) override;

	// 이동 차단 여부 확인 (노티파이에 의해 제어)
	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsMovementBlocked() const { return bAttackMovementBlocked; }

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

	// 유효한 SkeletalMeshComponent를 반환 (GetMesh() 실패 시 컴포넌트 검색)
	USkeletalMeshComponent* FindSkeletalMeshComponent() const;

	UPROPERTY()
	TObjectPtr<UBlendSpace1D> IdleRunBlendSpace;

	// 현재 재생 중인 블렌드 스페이스 추적용
	bool bIsPlayingBlendSpace = false;

	FVector PreviousAnimationLocation;
	bool bHasPreviousAnimationLocation;

	// 공격 중 플래그
	bool bIsAttacking = false;

	// 공격 중 이동 차단 플래그 (노티파이에 의해 제어)
	bool bAttackMovementBlocked = false;

	// 공격 입력 차단 플래그 (노티파이에 의해 제어)
	bool bAttackInputBlocked = true;

	// AI 타겟 액터
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;

	// 타겟까지 거리
	float DistanceToTarget = 0.0f;
};
