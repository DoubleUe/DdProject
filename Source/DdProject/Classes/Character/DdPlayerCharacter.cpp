#include "DdPlayerCharacter.h"

#include "Animation/AnimSequenceBase.h"
#include "Components/DdPlayerCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ADdPlayerCharacter::ADdPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement();
	CharacterMovementComponent->bOrientRotationToMovement = true;
	CharacterMovementComponent->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	CharacterMovementComponent->JumpZVelocity = 500.0f;
	CharacterMovementComponent->AirControl = 0.35f;
	CharacterMovementComponent->MaxWalkSpeed = 500.0f;
	CharacterMovementComponent->MinAnalogWalkSpeed = 20.0f;
	CharacterMovementComponent->BrakingDecelerationWalking = 2000.0f;
	CharacterMovementComponent->BrakingDecelerationFalling = 1500.0f;

	PlayerCameraComp = CreateDefaultSubobject<UDdPlayerCameraComponent>(TEXT("PlayerCameraComp"));

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -97.0f), FRotator(0.0f, -90.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Player/Character/Mesh/SK_Mannequin.SK_Mannequin"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> AttackAnimationAsset(TEXT("/Game/Characters/Player/Animations/Standing_Melee_Attack_Downward.Standing_Melee_Attack_Downward"));
	if (AttackAnimationAsset.Succeeded())
	{
		AttackAnimation = AttackAnimationAsset.Object;
	}
}

void ADdPlayerCharacter::SetAttackMovementInputBlocked(bool bBlocked)
{
	bAttackMovementInputBlocked = bBlocked;
}

void ADdPlayerCharacter::SetAttackInputBlocked(bool bBlocked)
{
	bAttackInputBlocked = bBlocked;
}

void ADdPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PlayerCameraComp != nullptr)
	{
		PlayerCameraComp->UpdateCameraZoom(DeltaSeconds);
	}
}

void ADdPlayerCharacter::ApplyCameraZoomInput(const FInputActionValue& Value)
{
	if (PlayerCameraComp != nullptr)
	{
		PlayerCameraComp->ZoomCamera(Value.Get<float>());
	}
}

void ADdPlayerCharacter::ApplyMoveInput(const FInputActionValue& Value)
{
	if (Controller == nullptr)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ADdPlayerCharacter::ApplyLookInput(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (!FMath::IsNearlyZero(LookAxisVector.X))
	{
		AddControllerYawInput(LookAxisVector.X);
	}

	const float PitchInput = -LookAxisVector.Y;
	if (!FMath::IsNearlyZero(PitchInput))
	{
		AddControllerPitchInput(PitchInput);
	}
}

void ADdPlayerCharacter::TryAttack()
{
	if (!CanProcessAttackInput() || bAttackAnimationPlaying || AttackAnimation == nullptr || GetMesh() == nullptr)
	{
		return;
	}

	const float AttackDuration = AttackAnimation->GetPlayLength();
	if (AttackDuration <= 0.0f)
	{
		return;
	}

	bAttackAnimationPlaying = true;
	bAttackMovementInputBlocked = true;
	GetWorldTimerManager().ClearTimer(AttackAnimationTimerHandle);
	GetMesh()->PlayAnimation(AttackAnimation, false);
	GetWorldTimerManager().SetTimer(
		AttackAnimationTimerHandle,
		this,
		&ADdPlayerCharacter::FinishAttackAnimation,
		AttackDuration,
		false);
}

void ADdPlayerCharacter::FinishAttackAnimation()
{
	bAttackAnimationPlaying = false;
}

bool ADdPlayerCharacter::CanProcessAttackInput() const
{
	return !bAttackInputBlocked;
}
