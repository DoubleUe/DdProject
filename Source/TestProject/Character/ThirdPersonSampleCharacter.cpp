#include "ThirdPersonSampleCharacter.h"

#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "UObject/ConstructorHelpers.h"

AThirdPersonSampleCharacter::AThirdPersonSampleCharacter()
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

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshAsset(TEXT("/Game/Characters/ThirdPerson/Character/Mesh/SK_Mannequin.SK_Mannequin"));
	if (MeshAsset.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(MeshAsset.Object);
	}

	static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBlueprintClass(TEXT("/Game/Characters/ThirdPerson/Animations/ThirdPerson_AnimBP"));
	if (AnimBlueprintClass.Class != nullptr)
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(AnimBlueprintClass.Class);
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionAsset(TEXT("/Game/Characters/ThirdPerson/Input/Actions/IA_Jump.IA_Jump"));
	if (JumpActionAsset.Succeeded())
	{
		JumpAction = JumpActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(TEXT("/Game/Characters/ThirdPerson/Input/Actions/IA_Move.IA_Move"));
	if (MoveActionAsset.Succeeded())
	{
		MoveAction = MoveActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(TEXT("/Game/Characters/ThirdPerson/Input/Actions/IA_Look.IA_Look"));
	if (LookActionAsset.Succeeded())
	{
		LookAction = LookActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MouseLookActionAsset(TEXT("/Game/Characters/ThirdPerson/Input/Actions/IA_MouseLook.IA_MouseLook"));
	if (MouseLookActionAsset.Succeeded())
	{
		MouseLookAction = MouseLookActionAsset.Object;
	}
}

void AThirdPersonSampleCharacter::BeginPlay()
{
	Super::BeginPlay();

	DesiredCameraBoomLength = CameraBoom != nullptr ? CameraBoom->TargetArmLength : DesiredCameraBoomLength;
	RefreshCameraZoomBounds();
}

void AThirdPersonSampleCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AThirdPersonSampleCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AThirdPersonSampleCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AThirdPersonSampleCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AThirdPersonSampleCharacter::LookUp);
	PlayerInputComponent->BindAxis(TEXT("CameraZoom"), this, &AThirdPersonSampleCharacter::ZoomCamera);
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
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AThirdPersonSampleCharacter::Move);
	}

	if (LookAction != nullptr)
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThirdPersonSampleCharacter::Look);
	}

	if (MouseLookAction != nullptr)
	{
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AThirdPersonSampleCharacter::Look);
	}
}

void AThirdPersonSampleCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CameraBoom == nullptr)
	{
		return;
	}

	RefreshCameraZoomBounds();

	const float NewArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		DesiredCameraBoomLength,
		DeltaSeconds,
		CameraZoomInterpSpeed);

	CameraBoom->TargetArmLength = FMath::Clamp(NewArmLength, MinCameraBoomLength, MaxCameraBoomLength);
}

void AThirdPersonSampleCharacter::MoveForward(float Value)
{
	if (FMath::IsNearlyZero(Value) || Controller == nullptr)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, Value);
}

void AThirdPersonSampleCharacter::MoveRight(float Value)
{
	if (FMath::IsNearlyZero(Value) || Controller == nullptr)
	{
		return;
	}

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, Value);
}

void AThirdPersonSampleCharacter::Turn(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddControllerYawInput(Value);
	}
}

void AThirdPersonSampleCharacter::LookUp(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddControllerPitchInput(-Value);
	}
}

void AThirdPersonSampleCharacter::ZoomCamera(float Value)
{
	if (FMath::IsNearlyZero(Value) || CameraBoom == nullptr)
	{
		return;
	}

	RefreshCameraZoomBounds();
	DesiredCameraBoomLength = FMath::Clamp(
		DesiredCameraBoomLength - (Value * CameraZoomStep),
		MinCameraBoomLength,
		MaxCameraBoomLength);
}

void AThirdPersonSampleCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	MoveForward(MovementVector.Y);
	MoveRight(MovementVector.X);
}

void AThirdPersonSampleCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	Turn(LookAxisVector.X);
	LookUp(LookAxisVector.Y);
}

void AThirdPersonSampleCharacter::RefreshCameraZoomBounds()
{
	if (CameraBoom == nullptr)
	{
		return;
	}

	const float CapsuleZoomLimit = GetCapsuleComponent()->GetScaledCapsuleRadius() * 4.0f;
	MinCameraBoomLength = FMath::Max(0.0f, CapsuleZoomLimit);
	MaxCameraBoomLength = FMath::Max(MinCameraBoomLength, MaxCameraBoomLength);
	DesiredCameraBoomLength = FMath::Clamp(DesiredCameraBoomLength, MinCameraBoomLength, MaxCameraBoomLength);
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength, MinCameraBoomLength, MaxCameraBoomLength);
}
