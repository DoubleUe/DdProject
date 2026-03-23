#include "MonsterCharacter.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimationAsset.h"
#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/ConstructorHelpers.h"

AMonsterCharacter::AMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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
	static ConstructorHelpers::FObjectFinder<UAnimationAsset> ZombieIdleAnimationAsset(TEXT("/Game/Characters/Zombie/Animation/Zombie_Idle.Zombie_Idle"));
	static ConstructorHelpers::FObjectFinder<UAnimationAsset> ZombieRunAnimationAsset(TEXT("/Game/Characters/Zombie/Animation/Zombie_Running_Anim.Zombie_Running_Anim"));
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

	IdleAnimation = ZombieIdleAnimationAsset.Succeeded() ? ZombieIdleAnimationAsset.Object : nullptr;
	RunAnimation = ZombieRunAnimationAsset.Succeeded() ? ZombieRunAnimationAsset.Object : nullptr;
	CurrentLoopAnimation = nullptr;
}

void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateMovementAnimation();
}

void AMonsterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateMovementAnimation();
}

void AMonsterCharacter::UpdateMovementAnimation()
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent == nullptr)
	{
		return;
	}

	const float Speed2D = GetVelocity().Size2D();
	UAnimationAsset* DesiredAnimation = Speed2D > 3.0f ? RunAnimation : IdleAnimation;
	if (DesiredAnimation == nullptr || CurrentLoopAnimation == DesiredAnimation)
	{
		return;
	}

	MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	MeshComponent->PlayAnimation(DesiredAnimation, true);
	CurrentLoopAnimation = DesiredAnimation;
}
