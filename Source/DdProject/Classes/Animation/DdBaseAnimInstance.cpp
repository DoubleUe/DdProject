#include "Animation/DdBaseAnimInstance.h"

#include "Character/DdBaseCharacter.h"
#include "CharacterTrajectoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UDdBaseAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CacheOwnerComponents();
	UpdateMovementState();
}

void UDdBaseAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	CacheOwnerComponents();
	UpdateMovementState();
}

void UDdBaseAnimInstance::CacheOwnerComponents()
{
	ADdBaseCharacter* BaseCharacter = Cast<ADdBaseCharacter>(TryGetPawnOwner());

	if (!IsValid(BaseCharacter))
	{
		TrajectoryComponent = nullptr;
		CharacterMovementComponent = nullptr;
		return;
	}

	if (!IsValid(TrajectoryComponent))
	{
		TrajectoryComponent = BaseCharacter->GetTrajectoryComponent();
	}

	if (!IsValid(CharacterMovementComponent))
	{
		CharacterMovementComponent = BaseCharacter->GetCharacterMovement();
	}
}

void UDdBaseAnimInstance::UpdateMovementState()
{
	if (!IsValid(CharacterMovementComponent))
	{
		Speed = 0.0f;
		bIsFalling = false;
		return;
	}

	Speed = CharacterMovementComponent->Velocity.Size();
	bIsFalling = CharacterMovementComponent->IsFalling();
}
