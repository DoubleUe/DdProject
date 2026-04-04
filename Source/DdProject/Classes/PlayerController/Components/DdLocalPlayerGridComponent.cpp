#include "DdLocalPlayerGridComponent.h"

#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UDdLocalPlayerGridComponent::UDdLocalPlayerGridComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UDdLocalPlayerGridComponent::SetGridVisible(bool bVisible)
{
	if (bGridVisible == bVisible)
	{
		return;
	}

	bGridVisible = bVisible;
	SetComponentTickEnabled(bGridVisible);
}

void UDdLocalPlayerGridComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bGridVisible)
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (PlayerController == nullptr || !PlayerController->IsLocalController())
	{
		return;
	}

	const APawn* ControlledPawn = PlayerController->GetPawn();
	if (ControlledPawn == nullptr)
	{
		return;
	}

	FVector GridCenter = ControlledPawn->GetActorLocation();
	if (const ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn))
	{
		if (const UCapsuleComponent* CapsuleComponent = ControlledCharacter->GetCapsuleComponent())
		{
			GridCenter.Z -= CapsuleComponent->GetScaledCapsuleHalfHeight();
		}
	}

	DrawLocalGrid(GridCenter);
}

void UDdLocalPlayerGridComponent::DrawLocalGrid(const FVector& Center) const
{
	UWorld* World = GetWorld();
	if (World == nullptr || GridStep <= 0.0f || GridSize <= 0.0f)
	{
		return;
	}

	const float HalfExtent = GridSize * 0.5f;
	const float MinX = Center.X - HalfExtent;
	const float MaxX = Center.X + HalfExtent;
	const float MinY = Center.Y - HalfExtent;
	const float MaxY = Center.Y + HalfExtent;
	const float PlaneZ = Center.Z;

	const int32 StartXIndex = FMath::FloorToInt(MinX / GridStep);
	const int32 EndXIndex = FMath::CeilToInt(MaxX / GridStep);
	for (int32 XIndex = StartXIndex; XIndex <= EndXIndex; ++XIndex)
	{
		const float X = static_cast<float>(XIndex) * GridStep;
		DrawDebugLine(
			World,
			FVector(X, MinY, PlaneZ),
			FVector(X, MaxY, PlaneZ),
			GridColor,
			false,
			0.0f,
			0,
			LineThickness);
	}

	const int32 StartYIndex = FMath::FloorToInt(MinY / GridStep);
	const int32 EndYIndex = FMath::CeilToInt(MaxY / GridStep);
	for (int32 YIndex = StartYIndex; YIndex <= EndYIndex; ++YIndex)
	{
		const float Y = static_cast<float>(YIndex) * GridStep;
		DrawDebugLine(
			World,
			FVector(MinX, Y, PlaneZ),
			FVector(MaxX, Y, PlaneZ),
			GridColor,
			false,
			0.0f,
			0,
			LineThickness);
	}
}
