#include "PlayerCharacter.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimSequenceBase.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

APlayerCharacter::APlayerCharacter()
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

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	MinCameraBoomLength = GetCapsuleComponent()->GetUnscaledCapsuleRadius() * 2.0f;
	MaxCameraBoomLength = CameraBoom->TargetArmLength * 2.0f;
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength, MinCameraBoomLength, MaxCameraBoomLength);
	DesiredCameraBoomLength = CameraBoom->TargetArmLength;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -97.0f), FRotator(0.0f, -90.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/Player/Character/Mesh/SK_Mannequin.SK_Mannequin"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBlueprintClass(TEXT("/Game/Characters/Player/Animations/ThirdPerson_AnimBP"));
	if (AnimBlueprintClass.Class != nullptr)
	{
		CharacterAnimBlueprintClass = AnimBlueprintClass.Class;
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(CharacterAnimBlueprintClass);
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionAsset(TEXT("/Game/Characters/Player/Input/Actions/IA_Jump.IA_Jump"));
	if (JumpActionAsset.Succeeded())
	{
		JumpAction = JumpActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(TEXT("/Game/Characters/Player/Input/Actions/IA_Move.IA_Move"));
	if (MoveActionAsset.Succeeded())
	{
		MoveAction = MoveActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(TEXT("/Game/Characters/Player/Input/Actions/IA_Look.IA_Look"));
	if (LookActionAsset.Succeeded())
	{
		LookAction = LookActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookActionAsset(TEXT("/Game/Characters/Player/Input/Actions/IA_MouseLook.IA_MouseLook"));
	if (MouseLookActionAsset.Succeeded())
	{
		MouseLookAction = MouseLookActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UAnimSequenceBase> AttackAnimationAsset(TEXT("/Game/Characters/Player/Animations/Standing_Melee_Attack_Downward.Standing_Melee_Attack_Downward"));
	if (AttackAnimationAsset.Succeeded())
	{
		AttackAnimation = AttackAnimationAsset.Object;
	}
}

void APlayerCharacter::SetAttackMovementInputBlocked(bool bBlocked)
{
	bAttackMovementInputBlocked = bBlocked;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	DesiredCameraBoomLength = CameraBoom != nullptr ? CameraBoom->TargetArmLength : DesiredCameraBoomLength;
	bCameraZoomBoundsDirty = true;
	UpdateCameraZoom(0.0f);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &APlayerCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &APlayerCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("CameraZoom"), this, &APlayerCharacter::ZoomCamera);
	PlayerInputComponent->BindAction(TEXT("Attack"), IE_Pressed, this, &APlayerCharacter::Attack);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);

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
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
	}

	if (LookAction != nullptr)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
	}

	if (MouseLookAction != nullptr)
	{
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
	}
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCameraZoom(DeltaSeconds);
}

void APlayerCharacter::MoveForward(float Value)
{
	if (FMath::IsNearlyZero(Value) || Controller == nullptr || !CanProcessMovementInput())
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, Value);
}

void APlayerCharacter::MoveRight(float Value)
{
	if (FMath::IsNearlyZero(Value) || Controller == nullptr || !CanProcessMovementInput())
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, Value);
}

void APlayerCharacter::Turn(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddControllerYawInput(Value);
	}
}

void APlayerCharacter::LookUp(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddControllerPitchInput(-Value);
	}
}

void APlayerCharacter::ZoomCamera(float Value)
{
	if (FMath::IsNearlyZero(Value) || CameraBoom == nullptr)
	{
		return;
	}

	DesiredCameraBoomLength -= Value * CameraZoomStep;
	bCameraZoomBoundsDirty = true;
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	MoveForward(MovementVector.Y);
	MoveRight(MovementVector.X);
}

void APlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	Turn(LookAxisVector.X);
	LookUp(LookAxisVector.Y);
}

void APlayerCharacter::Attack()
{
	if (bAttackAnimationPlaying || AttackAnimation == nullptr || GetMesh() == nullptr)
	{
		return;
	}

	const float AttackDuration = AttackAnimation->GetPlayLength();
	if (AttackDuration <= 0.0f)
	{
		return;
	}

	bAttackAnimationPlaying = true;
	bAttackMovementInputBlocked = false;
	GetWorldTimerManager().ClearTimer(AttackAnimationTimerHandle);
	GetMesh()->PlayAnimation(AttackAnimation, false);
	GetWorldTimerManager().SetTimer(
		AttackAnimationTimerHandle,
		this,
		&APlayerCharacter::FinishAttackAnimation,
		AttackDuration,
		false);
}

void APlayerCharacter::FinishAttackAnimation()
{
	bAttackAnimationPlaying = false;
	bAttackMovementInputBlocked = false;

	if (GetMesh() == nullptr || CharacterAnimBlueprintClass == nullptr)
	{
		return;
	}

	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(CharacterAnimBlueprintClass);
}

bool APlayerCharacter::CanProcessMovementInput() const
{
	return !bAttackAnimationPlaying || !bAttackMovementInputBlocked;
}

void APlayerCharacter::UpdateCameraZoom(float DeltaSeconds)
{
	if (CameraBoom == nullptr)
	{
		return;
	}

	const float CapsuleZoomLimit = FMath::Max(0.0f, GetCapsuleComponent()->GetScaledCapsuleRadius() * 4.0f);
	if (!FMath::IsNearlyEqual(CachedCapsuleZoomLimit, CapsuleZoomLimit))
	{
		CachedCapsuleZoomLimit = CapsuleZoomLimit;
		bCameraZoomBoundsDirty = true;
	}

	if (bCameraZoomBoundsDirty)
	{
		RefreshCameraZoomBounds();
		bCameraZoomBoundsDirty = false;
	}

	if (FMath::IsNearlyEqual(CameraBoom->TargetArmLength, DesiredCameraBoomLength))
	{
		return;
	}

	const float NewArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		DesiredCameraBoomLength,
		DeltaSeconds,
		CameraZoomInterpSpeed);

	CameraBoom->TargetArmLength = FMath::IsNearlyEqual(NewArmLength, DesiredCameraBoomLength)
		? DesiredCameraBoomLength
		: NewArmLength;
}

void APlayerCharacter::RefreshCameraZoomBounds()
{
	if (CameraBoom == nullptr)
	{
		return;
	}

	const float CapsuleZoomLimit = CachedCapsuleZoomLimit >= 0.0f
		? CachedCapsuleZoomLimit
		: FMath::Max(0.0f, GetCapsuleComponent()->GetScaledCapsuleRadius() * 4.0f);
	MinCameraBoomLength = FMath::Max(0.0f, CapsuleZoomLimit);
	MaxCameraBoomLength = FMath::Max(MinCameraBoomLength, MaxCameraBoomLength);
	DesiredCameraBoomLength = FMath::Clamp(DesiredCameraBoomLength, MinCameraBoomLength, MaxCameraBoomLength);
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength, MinCameraBoomLength, MaxCameraBoomLength);
}
