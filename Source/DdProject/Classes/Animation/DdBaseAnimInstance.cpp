#include "Animation/DdBaseAnimInstance.h"

#include "Character/DdBaseCharacter.h"
#include "CharacterTrajectoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UDdBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CacheOwnerComponents();
}

void UDdBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	CacheOwnerComponents();
}

void UDdBaseAnimInstance::CacheOwnerComponents()
{
	ADdBaseCharacter* BaseCharacter = Cast<ADdBaseCharacter>(TryGetPawnOwner());

	if (!BaseCharacter)
	{
		TrajectoryComponent = nullptr;
		CharacterMovementComponent = nullptr;
		return;
	}

	TrajectoryComponent = BaseCharacter->GetTrajectoryComponent();
	CharacterMovementComponent = BaseCharacter->GetCharacterMovement();
}
