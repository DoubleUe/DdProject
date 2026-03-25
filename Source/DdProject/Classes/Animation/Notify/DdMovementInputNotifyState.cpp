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
	SetMovementBlocked(MeshComp, true);
}

void UDdMovementInputNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	SetMovementBlocked(MeshComp, false);
}
