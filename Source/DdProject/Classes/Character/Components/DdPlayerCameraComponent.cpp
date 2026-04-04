#include "DdPlayerCameraComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

UDdPlayerCameraComponent::UDdPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDdPlayerCameraComponent::OnRegister()
{
	Super::OnRegister();

	CreateCameraComponents();
}

void UDdPlayerCameraComponent::CreateCameraComponents()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || CameraBoom != nullptr)
	{
		return;
	}

	USceneComponent* RootComp = Owner->GetRootComponent();
	if (RootComp == nullptr)
	{
		return;
	}

	CameraBoom = NewObject<USpringArmComponent>(Owner, TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComp);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->RegisterComponent();

	ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
	if (OwnerCharacter != nullptr && OwnerCharacter->GetCapsuleComponent() != nullptr)
	{
		MinCameraBoomLength = OwnerCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius() * 2.0f;
	}
	MaxCameraBoomLength = CameraBoom->TargetArmLength * 2.0f;
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength, MinCameraBoomLength, MaxCameraBoomLength);
	DesiredCameraBoomLength = CameraBoom->TargetArmLength;

	FollowCamera = NewObject<UCameraComponent>(Owner, TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->RegisterComponent();
}

void UDdPlayerCameraComponent::ZoomCamera(float Value)
{
	if (FMath::IsNearlyZero(Value) || CameraBoom == nullptr)
	{
		return;
	}

	DesiredCameraBoomLength -= Value * CameraZoomStep;
	bCameraZoomBoundsDirty = true;
}

void UDdPlayerCameraComponent::UpdateCameraZoom(float DeltaSeconds)
{
	if (CameraBoom == nullptr)
	{
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	if (OwnerCharacter != nullptr && OwnerCharacter->GetCapsuleComponent() != nullptr)
	{
		const float CapsuleZoomLimit = FMath::Max(0.0f, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius() * 4.0f);
		if (!FMath::IsNearlyEqual(CachedCapsuleZoomLimit, CapsuleZoomLimit))
		{
			CachedCapsuleZoomLimit = CapsuleZoomLimit;
			bCameraZoomBoundsDirty = true;
		}
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

void UDdPlayerCameraComponent::RefreshCameraZoomBounds()
{
	if (CameraBoom == nullptr)
	{
		return;
	}

	const float CapsuleZoomLimit = CachedCapsuleZoomLimit >= 0.0f
		? CachedCapsuleZoomLimit
		: 0.0f;
	MinCameraBoomLength = FMath::Max(0.0f, CapsuleZoomLimit);
	MaxCameraBoomLength = FMath::Max(MinCameraBoomLength, MaxCameraBoomLength);
	DesiredCameraBoomLength = FMath::Clamp(DesiredCameraBoomLength, MinCameraBoomLength, MaxCameraBoomLength);
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength, MinCameraBoomLength, MaxCameraBoomLength);
}
