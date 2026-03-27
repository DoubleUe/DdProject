#include "DdBaseCharacter.h"

#include "CharacterTrajectoryComponent.h"

ADdBaseCharacter::ADdBaseCharacter()
{
	TrajectoryComponent = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("TrajectoryComponent"));
}

UCharacterTrajectoryComponent* ADdBaseCharacter::GetTrajectoryComponent() const
{
	return TrajectoryComponent;
}
