#include "MonsterCharacter.h"

#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

AMonsterCharacter::AMonsterCharacter()
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

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -96.0f), FRotator(0.0f, -90.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> ZombieMeshAsset(TEXT("/Game/Characters/Zombie/Mesh/Zombie_Running.Zombie_Running"));
	static ConstructorHelpers::FObjectFinder<USkeleton> ZombieSkeletonAsset(TEXT("/Game/Characters/Zombie/Mesh/Zombie_Running_Skeleton.Zombie_Running_Skeleton"));
	if (ZombieMeshAsset.Succeeded())
	{
		USkeletalMesh* ZombieMesh = ZombieMeshAsset.Object;
		if (ZombieSkeletonAsset.Succeeded() && ZombieMesh->GetSkeleton() == ZombieSkeletonAsset.Object)
		{
			GetMesh()->SetSkeletalMesh(ZombieMesh);
		}
		else
		{
			GetMesh()->SetSkeletalMesh(ZombieMesh);
		}
	}
}
