#include "DdBaseCharacter.h"

#include "CharacterTrajectoryComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ADdBaseCharacter::ADdBaseCharacter()
{
	bReplicates = true;

	TrajectoryComponent = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("TrajectoryComponent"));

	ApplyCameraCollisionIgnores();
	ApplyRotationModeFromState();
	ApplyWalkSpeedFromState();
}

void ADdBaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	ApplyCameraCollisionIgnores();
}

void ADdBaseCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ApplyCameraCollisionIgnores();
}

UCharacterTrajectoryComponent* ADdBaseCharacter::GetTrajectoryComponent() const
{
	return TrajectoryComponent;
}

void ADdBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADdBaseCharacter, bUseControllerDesiredRotationMode);
	DOREPLIFETIME(ADdBaseCharacter, bUseSlowWalkSpeed);
}

void ADdBaseCharacter::SetMovementInputBlocked(bool bBlocked)
{
	bMovementInputBlocked = bBlocked;
}

void ADdBaseCharacter::SetAttackInputBlocked(bool bBlocked)
{
	bAttackInputBlocked = bBlocked;
}

void ADdBaseCharacter::ToggleRotationMode()
{
	const bool bNextUseControllerDesiredRotationMode = !bUseControllerDesiredRotationMode;
	SetUseControllerDesiredRotationMode(bNextUseControllerDesiredRotationMode);

	if (!HasAuthority())
	{
		ServerSetUseControllerDesiredRotationMode(bNextUseControllerDesiredRotationMode);
	}
}

void ADdBaseCharacter::ToggleWalkSpeed()
{
	const bool bNextUseSlowWalkSpeed = !bUseSlowWalkSpeed;
	SetUseSlowWalkSpeed(bNextUseSlowWalkSpeed);

	if (!HasAuthority())
	{
		ServerSetUseSlowWalkSpeed(bNextUseSlowWalkSpeed);
	}
}

void ADdBaseCharacter::ApplyCameraCollisionIgnores()
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (PrimitiveComponent == nullptr || PrimitiveComponent->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
}

void ADdBaseCharacter::ApplyWalkSpeedFromState()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	CharacterMovementComponent->MaxWalkSpeed = bUseSlowWalkSpeed ? SlowWalkSpeed : NormalWalkSpeed;
}

void ADdBaseCharacter::SetUseSlowWalkSpeed(bool bInUseSlowWalkSpeed)
{
	bUseSlowWalkSpeed = bInUseSlowWalkSpeed;
	ApplyWalkSpeedFromState();
}

void ADdBaseCharacter::ApplyRotationModeFromState()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	CharacterMovementComponent->bOrientRotationToMovement = !bUseControllerDesiredRotationMode;
	CharacterMovementComponent->bUseControllerDesiredRotation = bUseControllerDesiredRotationMode;
}

void ADdBaseCharacter::SetUseControllerDesiredRotationMode(bool bInUseControllerDesiredRotationMode)
{
	bUseControllerDesiredRotationMode = bInUseControllerDesiredRotationMode;
	ApplyRotationModeFromState();
}

void ADdBaseCharacter::OnRep_UseControllerDesiredRotationMode()
{
	ApplyRotationModeFromState();
}

void ADdBaseCharacter::OnRep_UseSlowWalkSpeed()
{
	ApplyWalkSpeedFromState();
}

void ADdBaseCharacter::ServerSetUseControllerDesiredRotationMode_Implementation(bool bInUseControllerDesiredRotationMode)
{
	SetUseControllerDesiredRotationMode(bInUseControllerDesiredRotationMode);
}

void ADdBaseCharacter::ServerSetUseSlowWalkSpeed_Implementation(bool bInUseSlowWalkSpeed)
{
	SetUseSlowWalkSpeed(bInUseSlowWalkSpeed);
}
