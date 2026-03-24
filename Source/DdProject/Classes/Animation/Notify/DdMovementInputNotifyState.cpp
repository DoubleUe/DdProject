#include "DdMovementInputNotifyState.h"

#include "../../Character/DdPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"

FString UDdMovementInputNotifyState::GetNotifyName_Implementation() const
{
	return TEXT("Movement Input State");
}

void UDdMovementInputNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	ADdPlayerCharacter* DdPlayerCharacter = Cast<ADdPlayerCharacter>(MeshComp->GetOwner());
	if (DdPlayerCharacter == nullptr)
	{
		return;
	}

	DdPlayerCharacter->SetAttackMovementInputBlocked(true);
}

void UDdMovementInputNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (MeshComp == nullptr)
	{
		return;
	}

	ADdPlayerCharacter* DdPlayerCharacter = Cast<ADdPlayerCharacter>(MeshComp->GetOwner());
	if (DdPlayerCharacter == nullptr)
	{
		return;
	}

	DdPlayerCharacter->SetAttackMovementInputBlocked(false);
}
