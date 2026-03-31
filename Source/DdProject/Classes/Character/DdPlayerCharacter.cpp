#include "DdPlayerCharacter.h"

#include "Animation/AnimSequenceBase.h"
#include "Component/DdPlayerCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"
#include "../PlayerController/DdGameplayPlayerController.h"

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

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionAsset(TEXT("/Game/Design/Input/Actions/IA_Jump.IA_Jump"));
	if (JumpActionAsset.Succeeded())
	{
		JumpAction = JumpActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(TEXT("/Game/Design/Input/Actions/IA_Move.IA_Move"));
	if (MoveActionAsset.Succeeded())
	{
		MoveAction = MoveActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(TEXT("/Game/Design/Input/Actions/IA_Look.IA_Look"));
	if (LookActionAsset.Succeeded())
	{
		LookAction = LookActionAsset.Object;
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
	bCanTransitionFromAttackToMovement = !bBlocked;
}

void ADdPlayerCharacter::SetAttackInputBlocked(bool bBlocked)
{
	bAttackInputBlocked = bBlocked;
}

void ADdPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		return;
	}

	if (JumpAction != nullptr)
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}

	if (MoveAction != nullptr)
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ADdPlayerCharacter::Move);
	}

	if (LookAction != nullptr)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ADdPlayerCharacter::Look);
	}

	if (const ADdGameplayPlayerController* GameplayPlayerController = Cast<ADdGameplayPlayerController>(GetController()))
	{
		if (const UInputAction* AttackAction = GameplayPlayerController->GetGameplayAttackAction())
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ADdPlayerCharacter::Attack);
		}

		if (const UInputAction* CameraZoomAction = GameplayPlayerController->GetGameplayCameraZoomAction())
		{
			EnhancedInputComponent->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &ADdPlayerCharacter::CameraZoom);
		}

		if (const UInputAction* ToggleRotationModeAction = GameplayPlayerController->GetGameplayToggleRotationModeAction())
		{
			EnhancedInputComponent->BindAction(ToggleRotationModeAction, ETriggerEvent::Started, this, &ADdBaseCharacter::ToggleRotationMode);
		}

		if (const UInputAction* ToggleWalkSpeedAction = GameplayPlayerController->GetGameplayToggleWalkSpeedAction())
		{
			EnhancedInputComponent->BindAction(ToggleWalkSpeedAction, ETriggerEvent::Started, this, &ADdBaseCharacter::ToggleWalkSpeed);
		}
	}
}

void ADdPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (PlayerCameraComp != nullptr)
	{
		PlayerCameraComp->UpdateCameraZoom(DeltaSeconds);
	}
}

void ADdPlayerCharacter::CameraZoom(const FInputActionValue& Value)
{
	if (PlayerCameraComp != nullptr)
	{
		PlayerCameraComp->ZoomCamera(Value.Get<float>());
	}
}

void ADdPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	/*if (Controller == nullptr || !CanProcessMovementInput())
	{
		return;
	}

	TryBlendToMovementAnimation();*/

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void ADdPlayerCharacter::Look(const FInputActionValue& Value)
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

void ADdPlayerCharacter::Attack()
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
	bCanTransitionFromAttackToMovement = false;
	GetWorldTimerManager().ClearTimer(AttackAnimationTimerHandle);
	GetMesh()->PlayAnimation(AttackAnimation, false);
	GetWorldTimerManager().SetTimer(
		AttackAnimationTimerHandle,
		this,
		&ADdPlayerCharacter::FinishAttackAnimation,
		AttackDuration,
		false);
}

void ADdPlayerCharacter::TryBlendToMovementAnimation()
{
	if (!bAttackAnimationPlaying || bAttackMovementInputBlocked || !bCanTransitionFromAttackToMovement)
	{
		return;
	}

	bAttackAnimationPlaying = false;
	bCanTransitionFromAttackToMovement = false;
	GetWorldTimerManager().ClearTimer(AttackAnimationTimerHandle);
}

void ADdPlayerCharacter::FinishAttackAnimation()
{
	bAttackAnimationPlaying = false;
	bCanTransitionFromAttackToMovement = false;
}

bool ADdPlayerCharacter::CanProcessMovementInput() const
{
	return !bAttackMovementInputBlocked;
}

bool ADdPlayerCharacter::CanProcessAttackInput() const
{
	return !bAttackInputBlocked;
}
