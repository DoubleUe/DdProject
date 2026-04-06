#include "DdPlayerCharacter.h"

#include "Components/DdCombatComponent.h"
#include "Components/DdPlayerCameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "InputActionValue.h"

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
}

void ADdPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
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
	if (Controller == nullptr || IsMovementInputBlocked())
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
	if (IsAttackBlocked())
	{
		return;
	}

	if (HasAuthority())
	{
		// 호스트: 서버에서 직접 실행
		ServerTryAttack_Implementation();
	}
	else
	{
		// 클라이언트: 서버에 공격 요청
		ServerTryAttack();
	}
}

void ADdPlayerCharacter::ServerTryAttack_Implementation()
{
	if (IsAttackBlocked() || CombatComponent == nullptr)
	{
		return;
	}

	const TArray<FDdComboSet>& ComboSets = CombatComponent->GetComboSets();
	if (ComboSets.Num() == 0 || ComboSets[0].Steps.Num() == 0)
	{
		return;
	}

	UAnimMontage* Montage = ComboSets[0].Steps[0].Montage;
	if (Montage == nullptr)
	{
		return;
	}

	MulticastPlayMontage(Montage);
}
