#include "DdBaseCharacter.h"

#include "CharacterTrajectoryComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ADdBaseCharacter::ADdBaseCharacter()
{
	TrajectoryComponent = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("TrajectoryComponent"));
}

UCharacterTrajectoryComponent* ADdBaseCharacter::GetTrajectoryComponent() const
{
	return TrajectoryComponent;
}

void ADdBaseCharacter::ToggleRotationMode()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	const bool bUseControllerDesiredRotation = CharacterMovementComponent->bOrientRotationToMovement;

	CharacterMovementComponent->bOrientRotationToMovement = !bUseControllerDesiredRotation;
	CharacterMovementComponent->bUseControllerDesiredRotation = bUseControllerDesiredRotation;
}
