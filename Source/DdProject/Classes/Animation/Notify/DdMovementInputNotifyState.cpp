#include "DdMovementInputNotifyState.h"

#include "../../Character/DdBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"

FString UDdMovementInputNotifyState::GetNotifyName_Implementation() const
{
	return TEXT("Movement Input State");
}

// 오너 액터에서 이동 차단 설정 (베이스 캐릭터 가상함수로 통합)
static void SetMovementBlocked(USkeletalMeshComponent* MeshComp, bool bBlocked)
{
	if (MeshComp == nullptr)
	{
		return;
	}

	if (ADdBaseCharacter* BaseCharacter = Cast<ADdBaseCharacter>(MeshComp->GetOwner()))
	{
		BaseCharacter->SetAttackMovementInputBlocked(bBlocked);
	}
}

void UDdMovementInputNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	// 노티파이 구간 시작 → 이동 허용
	SetMovementBlocked(MeshComp, false);
}

void UDdMovementInputNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	// 노티파이 구간 종료 → 이동 차단
	SetMovementBlocked(MeshComp, true);
}
