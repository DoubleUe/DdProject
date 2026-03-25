#include "DdAttackInputNotifyState.h"

#include "../../Character/DdBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"

FString UDdAttackInputNotifyState::GetNotifyName_Implementation() const
{
	return TEXT("Attack Input State");
}

static void SetAttackBlocked(USkeletalMeshComponent* MeshComp, bool bBlocked)
{
	if (MeshComp == nullptr)
	{
		return;
	}

	if (ADdBaseCharacter* BaseCharacter = Cast<ADdBaseCharacter>(MeshComp->GetOwner()))
	{
		BaseCharacter->SetAttackInputBlocked(bBlocked);
	}
}

void UDdAttackInputNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	// 노티파이 구간 시작 → 공격 허용
	SetAttackBlocked(MeshComp, false);
}

void UDdAttackInputNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	// 노티파이 구간 종료 → 공격 차단
	SetAttackBlocked(MeshComp, true);
}
