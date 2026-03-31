#include "DdBaseCharacter.h"

#include "CharacterTrajectoryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

ADdBaseCharacter::ADdBaseCharacter()
{
	bReplicates = true;

	TrajectoryComponent = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("TrajectoryComponent"));
	EquippedStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquippedStaticMeshComponent"));

	if (EquippedStaticMeshComponent != nullptr)
	{
		EquippedStaticMeshComponent->SetupAttachment(GetMesh());
		EquippedStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EquippedStaticMeshComponent->SetGenerateOverlapEvents(false);
		EquippedStaticMeshComponent->SetCanEverAffectNavigation(false);
	}

	ApplyRotationModeFromState();
	ApplyWalkSpeedFromState();
}

void ADdBaseCharacter::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	InitializeEquippedStaticMeshAttachment();
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

void ADdBaseCharacter::InitializeEquippedStaticMeshAttachment()
{
	USkeletalMeshComponent* CharacterMeshComponent = GetMesh();
	if (CharacterMeshComponent == nullptr || EquippedStaticMeshComponent == nullptr)
	{
		return;
	}

	if (!CharacterMeshComponent->DoesSocketExist(EquippedStaticMeshParentSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Missing parent socket '%s' on character mesh."), *GetName(), *EquippedStaticMeshParentSocketName.ToString());
		EquippedStaticMeshComponent->AttachToComponent(CharacterMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
		EquippedStaticMeshComponent->SetRelativeTransform(FTransform::Identity);
		return;
	}

	EquippedStaticMeshComponent->AttachToComponent(
		CharacterMeshComponent,
		FAttachmentTransformRules::KeepRelativeTransform,
		EquippedStaticMeshParentSocketName);

	if (EquippedStaticMeshComponent->GetStaticMesh() == nullptr)
	{
		EquippedStaticMeshComponent->SetRelativeTransform(FTransform::Identity);
		return;
	}

	if (!EquippedStaticMeshComponent->DoesSocketExist(EquippedStaticMeshChildSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: Missing child socket '%s' on equipped static mesh."), *GetName(), *EquippedStaticMeshChildSocketName.ToString());
		EquippedStaticMeshComponent->SetRelativeTransform(FTransform::Identity);
		return;
	}

	const FTransform ChildSocketComponentTransform = EquippedStaticMeshComponent->GetSocketTransform(
		EquippedStaticMeshChildSocketName,
		ERelativeTransformSpace::RTS_Component);

	EquippedStaticMeshComponent->SetRelativeTransform(ChildSocketComponentTransform.Inverse());
}
