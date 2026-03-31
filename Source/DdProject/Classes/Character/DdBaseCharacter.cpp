#include "DdBaseCharacter.h"

#include "CharacterTrajectoryComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ADdBaseCharacter::ADdBaseCharacter()
{
	TrajectoryComponent = CreateDefaultSubobject<UCharacterTrajectoryComponent>(TEXT("TrajectoryComponent"));
	EquippedStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EquippedStaticMeshComponent"));

	if (EquippedStaticMeshComponent != nullptr)
	{
		EquippedStaticMeshComponent->SetupAttachment(GetMesh());
		EquippedStaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EquippedStaticMeshComponent->SetGenerateOverlapEvents(false);
		EquippedStaticMeshComponent->SetCanEverAffectNavigation(false);
	}
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

void ADdBaseCharacter::ToggleWalkSpeed()
{
	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	if (CharacterMovementComponent == nullptr)
	{
		return;
	}

	bUseSlowWalkSpeed = !bUseSlowWalkSpeed;
	CharacterMovementComponent->MaxWalkSpeed = bUseSlowWalkSpeed ? SlowWalkSpeed : NormalWalkSpeed;
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
