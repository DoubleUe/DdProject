#include "DdMonsterCharacter.h"

#include "AIController.h"
#include "Components/DdCombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ADdMonsterCharacter::ADdMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	CharacterMovementComponent->bOrientRotationToMovement = true;
	CharacterMovementComponent->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	CharacterMovementComponent->MaxWalkSpeed = 300.0f;
	CharacterMovementComponent->BrakingDecelerationWalking = 2000.0f;

	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -96.0f), FRotator(0.0f, -90.0f, 0.0f));
}

void ADdMonsterCharacter::SetMovementInputBlocked(bool bBlocked)
{
	Super::SetMovementInputBlocked(bBlocked);

	if (bBlocked)
	{
		if (AAIController* AIC = Cast<AAIController>(GetController()))
		{
			AIC->StopMovement();
		}
		GetCharacterMovement()->DisableMovement();
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
}

bool ADdMonsterCharacter::PlayAttackAnimation()
{
	if (IsAttackBlocked() || CombatComponent == nullptr)
	{
		return false;
	}

	const TArray<FDdComboSet>& ComboSets = CombatComponent->GetComboSets();
	if (ComboSets.Num() == 0 || ComboSets[0].Steps.Num() == 0)
	{
		return false;
	}

	UAnimMontage* Montage = ComboSets[0].Steps[0].Montage;
	if (Montage == nullptr)
	{
		return false;
	}

	MulticastPlayMontage(Montage);
	return true;
}

