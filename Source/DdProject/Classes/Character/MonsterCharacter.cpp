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

	IdleRunBlendSpace = nullptr;
	IdleAnimation = nullptr;
	RunAnimation = nullptr;
	AttackAnimation = nullptr;
	CurrentLoopAnimation = nullptr;
	PreviousAnimationLocation = FVector::ZeroVector;
	bHasPreviousAnimationLocation = false;
	bIsAttacking = false;
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
}

void AMonsterCharacter::PlayAttackAnimation()
{
	// 이미 공격 중이면 무시
	if (bIsAttacking)
	{
		return;
	}

	if (AttackAnimation == nullptr)
	{
		return;
	}

	bIsAttacking = true;

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent == nullptr)
	{
		bIsAttacking = false;
		return;
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

	// 현재 루프 애니메이션 초기화 (공격 끝난 후 이동 애니메이션이 다시 설정되도록)
	CurrentLoopAnimation = nullptr;
}

void AMonsterCharacter::OnAttackAnimationEnded()
{
	bIsAttacking = false;
	CurrentLoopAnimation = nullptr;
}

void AMonsterCharacter::UpdateMovementAnimation(float DeltaSeconds)
{
	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent == nullptr)
	{
		return;
	}

	// 공격 애니메이션 재생 중에는 이동 애니메이션을 덮어쓰지 않음
	if (bIsAttacking)
	{
		// 공격 애니메이션이 끝났는지 확인
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
	const bool bIsMoving = Speed2D > 3.0f;

	if (!bIsMoving)
	{
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
