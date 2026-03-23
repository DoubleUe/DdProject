#include "MonsterCharacter.h"

#include "AIController.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
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

	static ConstructorHelpers::FClassFinder<AAIController> ZombieAiControllerClass(TEXT("/Game/Characters/Zombie/AI/BP_ZombieChaseAIController"));
	if (ZombieAiControllerClass.Succeeded())
	{
		AIControllerClass = ZombieAiControllerClass.Class;
	}
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

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

	IdleRunBlendSpace = nullptr;
	IdleAnimation = nullptr;
	RunAnimation = nullptr;
	CurrentLoopAnimation = nullptr;
	PreviousAnimationLocation = FVector::ZeroVector;
	bHasPreviousAnimationLocation = false;
	LastLoggedBlendSpeed = -1.0f;
}

void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	LoadAnimationAssets();
	UpdateMovementAnimation(0.0f);
}

void AMonsterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateMovementAnimation(DeltaSeconds);
}

void AMonsterCharacter::LoadAnimationAssets()
{
	IdleRunBlendSpace = LoadObject<UBlendSpace1D>(nullptr, TEXT("/Game/Characters/Zombie/Animation/Zombie_IdleRun_1D.Zombie_IdleRun_1D"));
	IdleAnimation = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Characters/Zombie/Animation/Zombie_Idle.Zombie_Idle"));
	RunAnimation = LoadObject<UAnimationAsset>(nullptr, TEXT("/Game/Characters/Zombie/Animation/Zombie_Running_Anim.Zombie_Running_Anim"));

	if (IdleRunBlendSpace != nullptr)
	{
		IdleRunBlendSpace->ConditionalPostLoad();
	}

	if (IdleAnimation != nullptr)
	{
		IdleAnimation->ConditionalPostLoad();
	}

	if (RunAnimation != nullptr)
	{
		RunAnimation->ConditionalPostLoad();
	}

	CurrentLoopAnimation = nullptr;
	LastLoggedBlendSpeed = -1.0f;
}

void AMonsterCharacter::UpdateMovementAnimation(float DeltaSeconds)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent == nullptr)
	{
		return;
	}

	const auto EnsureSingleNodeInstance = [MeshComponent]() -> UAnimSingleNodeInstance*
	{
		if (MeshComponent->GetAnimationMode() != EAnimationMode::AnimationSingleNode)
		{
			MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
			MeshComponent->InitAnim(true);
		}

		if (UAnimSingleNodeInstance* ExistingInstance = MeshComponent->GetSingleNodeInstance())
		{
			return ExistingInstance;
		}

		MeshComponent->InitAnim(true);
		return MeshComponent->GetSingleNodeInstance();
	};

	const FVector CurrentActorLocation = GetActorLocation();
	float LocationSpeed2D = 0.0f;
	if (bHasPreviousAnimationLocation && DeltaSeconds > UE_KINDA_SMALL_NUMBER)
	{
		LocationSpeed2D = FVector::Dist2D(CurrentActorLocation, PreviousAnimationLocation) / DeltaSeconds;
	}

	PreviousAnimationLocation = CurrentActorLocation;
	bHasPreviousAnimationLocation = true;

	const UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	const float ActorVelocitySpeed2D = GetVelocity().Size2D();
	const float MovementComponentSpeed2D = CharacterMovementComponent != nullptr ? CharacterMovementComponent->Velocity.Size2D() : 0.0f;
	const float Speed2D = FMath::Max(ActorVelocitySpeed2D, FMath::Max(MovementComponentSpeed2D, LocationSpeed2D));
	const bool bIsMoving = Speed2D > 3.0f;

	if (!bIsMoving)
	{
		LastLoggedBlendSpeed = -1.0f;

		if (IdleAnimation == nullptr || CurrentLoopAnimation == IdleAnimation)
		{
			return;
		}

		if (UAnimSingleNodeInstance* SingleNodeInstance = EnsureSingleNodeInstance())
		{
			SingleNodeInstance->SetAnimationAsset(IdleAnimation, true, 1.0f);
			SingleNodeInstance->SetPlaying(true);
			SingleNodeInstance->SetLooping(true);
			SingleNodeInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		}
		CurrentLoopAnimation = IdleAnimation;
		return;
	}

	if (IdleRunBlendSpace != nullptr)
	{
		if (UAnimSingleNodeInstance* SingleNodeInstance = EnsureSingleNodeInstance())
		{
			if (SingleNodeInstance->GetCurrentAsset() != IdleRunBlendSpace)
			{
				SingleNodeInstance->SetAnimationAsset(IdleRunBlendSpace, true, 1.0f);
			}

			const float BlendSpeed = FMath::Clamp(Speed2D, 0.0f, 300.0f);

#if WITH_EDITOR
			if (!FMath::IsNearlyEqual(LastLoggedBlendSpeed, BlendSpeed, 1.0f))
			{
				UE_LOG(LogTemp, Log, TEXT("%s BlendSpeed: %.2f"), *GetName(), BlendSpeed);
				LastLoggedBlendSpeed = BlendSpeed;
			}
#endif

			SingleNodeInstance->SetPlaying(true);
			SingleNodeInstance->SetLooping(true);
			SingleNodeInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
			SingleNodeInstance->SetBlendSpacePosition(FVector(BlendSpeed, 0.0f, 0.0f));
		}

		CurrentLoopAnimation = IdleRunBlendSpace;
		return;
	}

	UAnimationAsset* DesiredAnimation = RunAnimation;
	if (DesiredAnimation == nullptr || CurrentLoopAnimation == DesiredAnimation)
	{
		return;
	}

	if (UAnimSingleNodeInstance* SingleNodeInstance = EnsureSingleNodeInstance())
	{
		SingleNodeInstance->SetAnimationAsset(DesiredAnimation, true, 1.0f);
		SingleNodeInstance->SetPlaying(true);
		SingleNodeInstance->SetLooping(true);
		SingleNodeInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}
	CurrentLoopAnimation = DesiredAnimation;
}
