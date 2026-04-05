#include "DdMovementBlockNotifyState.h"

#include "../../Character/DdBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"

FString UDdMovementBlockNotifyState::GetNotifyName_Implementation() const
{
	return TEXT("Movement Block State");
}

static void SetMovementBlockedState(USkeletalMeshComponent* MeshComp, bool bBlocked)
{
	if (MeshComp == nullptr)
	{
		return;
	}

	if (ADdBaseCharacter* BaseCharacter = Cast<ADdBaseCharacter>(MeshComp->GetOwner()))
	{
		BaseCharacter->SetMovementInputBlocked(bBlocked);
	}
}

void UDdMovementBlockNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	SetMovementBlockedState(MeshComp, true);
}

void UDdMovementBlockNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	SetMovementBlockedState(MeshComp, false);
}
