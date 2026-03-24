#include "PlayerCameraComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"

UPlayerCameraComponent::UPlayerCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPlayerCameraComponent::OnRegister()
{
	Super::OnRegister();

	CreateCameraComponents();
}

void UPlayerCameraComponent::CreateCameraComponents()
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

	// 카메라 붐 생성
	CameraBoom = NewObject<USpringArmComponent>(Owner, TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComp);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->RegisterComponent();

	// 줌 범위 초기화
	ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
	if (OwnerCharacter != nullptr && OwnerCharacter->GetCapsuleComponent() != nullptr)
	{
		MinCameraBoomLength = OwnerCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius() * 2.0f;
	}
	MaxCameraBoomLength = CameraBoom->TargetArmLength * 2.0f;
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength, MinCameraBoomLength, MaxCameraBoomLength);
	DesiredCameraBoomLength = CameraBoom->TargetArmLength;

	// 카메라 생성
	FollowCamera = NewObject<UCameraComponent>(Owner, TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->RegisterComponent();
}

void UPlayerCameraComponent::ZoomCamera(float Value)
{
	if (FMath::IsNearlyZero(Value) || CameraBoom == nullptr)
	{
		return;
	}

	DesiredCameraBoomLength -= Value * CameraZoomStep;
	bCameraZoomBoundsDirty = true;
}

void UPlayerCameraComponent::UpdateCameraZoom(float DeltaSeconds)
{
	if (CameraBoom == nullptr)
	{
		return;
	}

	// 캡슐 크기 변경 감지
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

void UPlayerCameraComponent::RefreshCameraZoomBounds()
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
