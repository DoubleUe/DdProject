#include "DdMonsterCharacter.h"

#include "AIController.h"
#include "Animation/AnimationAsset.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/Skeleton.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

ADdMonsterCharacter::ADdMonsterCharacter()
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

	AIControllerClass = nullptr;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -96.0f), FRotator(0.0f, -90.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MonsterMeshAsset(TEXT("/Game/Characters/Zombie/Mesh/Zombie_Running.Zombie_Running"));
	static ConstructorHelpers::FObjectFinder<USkeleton> MonsterSkeletonAsset(TEXT("/Game/Characters/Zombie/Mesh/Zombie_Running_Skeleton.Zombie_Running_Skeleton"));
	if (MonsterMeshAsset.Succeeded())
	{
		USkeletalMesh* MonsterMesh = MonsterMeshAsset.Object;
		if (MonsterSkeletonAsset.Succeeded() && MonsterMesh->GetSkeleton() == MonsterSkeletonAsset.Object)
		{
			GetMesh()->SetSkeletalMesh(MonsterMesh);
		}
		else
		{
			GetMesh()->SetSkeletalMesh(MonsterMesh);
		}
	}

	static ConstructorHelpers::FObjectFinder<UAnimationAsset> AttackAnimationAsset(TEXT("/Game/Characters/Zombie/Animation/Zombie_Punching_1.Zombie_Punching_1"));
	if (AttackAnimationAsset.Succeeded())
	{
		AttackAnimation = AttackAnimationAsset.Object;
	}

	IdleRunBlendSpace = nullptr;
	PreviousAnimationLocation = FVector::ZeroVector;
	bHasPreviousAnimationLocation = false;
	bIsPlayingBlendSpace = false;
}

void ADdMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	LoadAnimationAssets();
	UpdateMovementAnimation(0.0f);
}

void ADdMonsterCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateMovementAnimation(DeltaSeconds);
}

void ADdMonsterCharacter::LoadAnimationAssets()
{
	IdleRunBlendSpace = LoadObject<UBlendSpace1D>(nullptr, TEXT("/Game/Characters/Zombie/Animation/Zombie_IdleRun_1D.Zombie_IdleRun_1D"));

	if (IdleRunBlendSpace != nullptr)
	{
		IdleRunBlendSpace->ConditionalPostLoad();
	}

	bIsPlayingBlendSpace = false;
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
	if (!CanAttack())
	{
		return false;
	}

	if (AttackAnimation == nullptr)
	{
		return false;
	}

	SetAttacking(true);
	SetMovementInputBlocked(true);

	USkeletalMeshComponent* MeshComponent = FindSkeletalMeshComponent();
	if (MeshComponent == nullptr)
	{
		SetAttacking(false);
		return false;
	}

	if (MeshComponent->GetAnimationMode() != EAnimationMode::AnimationSingleNode)
	{
		MeshComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MeshComponent->InitAnim(true);
	}

	if (UAnimSingleNodeInstance* SingleNodeInstance = MeshComponent->GetSingleNodeInstance())
	{
		SingleNodeInstance->SetAnimationAsset(AttackAnimation, false, 1.0f);
		SingleNodeInstance->SetPlaying(true);
		SingleNodeInstance->SetLooping(false);
		SingleNodeInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}

	bIsPlayingBlendSpace = false;

	return true;
}

void ADdMonsterCharacter::OnAttackAnimationEnded()
{
	SetAttacking(false);
	bIsPlayingBlendSpace = false;

	if (IsAttackInputBlocked())
	{
		SetAttackInputBlocked(false);
	}

	if (IsMovementInputBlocked())
	{
		SetMovementInputBlocked(false);
	}
}

void ADdMonsterCharacter::UpdateMovementAnimation(float DeltaSeconds)
{
	USkeletalMeshComponent* MeshComponent = FindSkeletalMeshComponent();
	if (MeshComponent == nullptr || IdleRunBlendSpace == nullptr)
	{
		return;
	}

	if (IsAttacking() && !bIsPlayingBlendSpace)
	{
		if (UAnimSingleNodeInstance* SingleNodeInstance = MeshComponent->GetSingleNodeInstance())
		{
			if (!SingleNodeInstance->IsPlaying())
			{
				OnAttackAnimationEnded();
			}
			else
			{
				return;
			}
		}
		else
		{
			OnAttackAnimationEnded();
		}
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

	if (UAnimSingleNodeInstance* SingleNodeInstance = EnsureSingleNodeInstance())
	{
		if (!bIsPlayingBlendSpace)
		{
			SingleNodeInstance->SetAnimationAsset(IdleRunBlendSpace, true, 1.0f);
			bIsPlayingBlendSpace = true;
		}

		const float BlendSpeed = FMath::Clamp(Speed2D, 0.0f, 300.0f);

		SingleNodeInstance->SetPlaying(true);
		SingleNodeInstance->SetLooping(true);
		SingleNodeInstance->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
		SingleNodeInstance->SetBlendSpacePosition(FVector(BlendSpeed, 0.0f, 0.0f));
	}
}

USkeletalMeshComponent* ADdMonsterCharacter::FindSkeletalMeshComponent() const
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (MeshComp != nullptr && MeshComp->GetSkeletalMeshAsset() != nullptr)
	{
		return MeshComp;
	}

	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
	for (USkeletalMeshComponent* Comp : SkeletalMeshComponents)
	{
		if (Comp != nullptr && Comp->GetSkeletalMeshAsset() != nullptr)
		{
			return Comp;
		}
	}

	return MeshComp;
}
