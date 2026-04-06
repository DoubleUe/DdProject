#include "DdAttackBlockNotifyState.h"

#include "../../Character/DdBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"

FString UDdAttackBlockNotifyState::GetNotifyName_Implementation() const
{
	return TEXT("Attack Block State");
}

static void SetAttackBlockedState(USkeletalMeshComponent* MeshComp, bool bBlocked)
{
	if (MeshComp == nullptr)
	{
		return;
	}

	if (ADdBaseCharacter* BaseCharacter = Cast<ADdBaseCharacter>(MeshComp->GetOwner()))
	{
		BaseCharacter->SetAttackBlocked(bBlocked);
	}
}

void UDdAttackBlockNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	SetAttackBlockedState(MeshComp, true);
}

void UDdAttackBlockNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	SetAttackBlockedState(MeshComp, false);
}
