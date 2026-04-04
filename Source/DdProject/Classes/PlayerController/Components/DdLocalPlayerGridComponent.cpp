#include "DdLocalPlayerGridComponent.h"

#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UDdLocalPlayerGridComponent::UDdLocalPlayerGridComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDdLocalPlayerGridComponent::SetGridVisible(bool bVisible)
{
	bGridVisible = bVisible;
}

void UDdLocalPlayerGridComponent::DrawGrid(UCanvas* Canvas) const
{
	if (!bGridVisible || Canvas == nullptr || GridStep <= 0.0f || GridSize <= 0.0f || HeightSampleStep <= 0.0f || GroundTraceHalfHeight <= 0.0f)
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

	FCollisionQueryParams QueryParams(TEXT("LocalPlayerGridGroundTrace"), false);
	QueryParams.AddIgnoredActor(PlayerController);
	QueryParams.AddIgnoredActor(ControlledPawn);

	FVector GridCenter = ControlledPawn->GetActorLocation();
	if (const ACharacter* ControlledCharacter = Cast<ACharacter>(ControlledPawn))
	{
		if (const UCapsuleComponent* CapsuleComponent = ControlledCharacter->GetCapsuleComponent())
		{
			GridCenter.Z -= CapsuleComponent->GetScaledCapsuleHalfHeight();
		}
	}

	SampleGroundPoint(FVector2D(GridCenter.X, GridCenter.Y), GridCenter.Z, QueryParams, GridCenter);

	const float HalfExtent = GridSize * 0.5f;
	const float MinX = GridCenter.X - HalfExtent;
	const float MaxX = GridCenter.X + HalfExtent;
	const float MinY = GridCenter.Y - HalfExtent;
	const float MaxY = GridCenter.Y + HalfExtent;
	const float PlaneZ = GridCenter.Z;

	const int32 StartXIndex = FMath::FloorToInt(MinX / GridStep);
	const int32 EndXIndex = FMath::CeilToInt(MaxX / GridStep);
	for (int32 XIndex = StartXIndex; XIndex <= EndXIndex; ++XIndex)
	{
		const float X = static_cast<float>(XIndex) * GridStep;
		DrawSampledLine(
			Canvas,
			*PlayerController,
			QueryParams,
			FVector(X, MinY, PlaneZ),
			FVector(X, MaxY, PlaneZ),
			PlaneZ);
	}

	const int32 StartYIndex = FMath::FloorToInt(MinY / GridStep);
	const int32 EndYIndex = FMath::CeilToInt(MaxY / GridStep);
	for (int32 YIndex = StartYIndex; YIndex <= EndYIndex; ++YIndex)
	{
		const float Y = static_cast<float>(YIndex) * GridStep;
		DrawSampledLine(
			Canvas,
			*PlayerController,
			QueryParams,
			FVector(MinX, Y, PlaneZ),
			FVector(MaxX, Y, PlaneZ),
			PlaneZ);
	}
}

bool UDdLocalPlayerGridComponent::SampleGroundPoint(const FVector2D& GridLocation, float ReferenceZ, const FCollisionQueryParams& QueryParams, FVector& OutGroundPoint) const
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		OutGroundPoint = FVector(GridLocation.X, GridLocation.Y, ReferenceZ);
		return false;
	}

	const FVector TraceStart(GridLocation.X, GridLocation.Y, ReferenceZ + GroundTraceHalfHeight);
	const FVector TraceEnd(GridLocation.X, GridLocation.Y, ReferenceZ - GroundTraceHalfHeight);

	FHitResult HitResult;
	if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		OutGroundPoint = HitResult.ImpactPoint;
		return true;
	}

	OutGroundPoint = FVector(GridLocation.X, GridLocation.Y, ReferenceZ);
	return false;
}

void UDdLocalPlayerGridComponent::DrawSampledLine(UCanvas* Canvas, const APlayerController& PlayerController, const FCollisionQueryParams& QueryParams, const FVector& Start, const FVector& End, float ReferenceZ) const
{
	const float LineLength = FVector2D::Distance(FVector2D(Start.X, Start.Y), FVector2D(End.X, End.Y));
	const int32 SegmentCount = FMath::Max(1, FMath::CeilToInt(LineLength / HeightSampleStep));

	// Each segment endpoint is traced to the ground so the drawn line follows the terrain.
	FVector PreviousGroundPoint = FVector::ZeroVector;
	bool bHasPreviousGroundPoint = false;

	for (int32 SegmentIndex = 0; SegmentIndex <= SegmentCount; ++SegmentIndex)
	{
		const float Alpha = static_cast<float>(SegmentIndex) / static_cast<float>(SegmentCount);
		const FVector LinePoint = FMath::Lerp(Start, End, Alpha);

		FVector CurrentGroundPoint = LinePoint;
		SampleGroundPoint(FVector2D(LinePoint.X, LinePoint.Y), ReferenceZ, QueryParams, CurrentGroundPoint);

		if (bHasPreviousGroundPoint)
		{
			FVector2D PreviousScreenPoint = FVector2D::ZeroVector;
			FVector2D CurrentScreenPoint = FVector2D::ZeroVector;
			if (PlayerController.ProjectWorldLocationToScreen(PreviousGroundPoint, PreviousScreenPoint, true)
				&& PlayerController.ProjectWorldLocationToScreen(CurrentGroundPoint, CurrentScreenPoint, true))
			{
				Canvas->K2_DrawLine(PreviousScreenPoint, CurrentScreenPoint, LineThickness, FLinearColor(GridColor));
			}
		}

		PreviousGroundPoint = CurrentGroundPoint;
		bHasPreviousGroundPoint = true;
	}
}
