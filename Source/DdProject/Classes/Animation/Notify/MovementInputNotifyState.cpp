#include "MovementInputNotifyState.h"

#include "../../Character/DdPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

FString UMovementInputNotifyState::GetNotifyName_Implementation() const
{
	return TEXT("Movement Input State");
}

void UMovementInputNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	ADdPlayerCharacter* PlayerCharacter = Cast<ADdPlayerCharacter>(MeshComp->GetOwner());
	if (PlayerCharacter == nullptr)
	{
		return;
	}

	PlayerCharacter->SetAttackMovementInputBlocked(true);
}

void UMovementInputNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	ADdPlayerCharacter* PlayerCharacter = Cast<ADdPlayerCharacter>(MeshComp->GetOwner());
	if (PlayerCharacter == nullptr)
	{
		return;
	}

	PlayerCharacter->SetAttackMovementInputBlocked(false);
}
