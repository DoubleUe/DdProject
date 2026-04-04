#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DdLocalPlayerGridComponent.generated.h"

class UCanvas;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DDPROJECT_API UDdLocalPlayerGridComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDdLocalPlayerGridComponent();

	void SetGridVisible(bool bVisible);
	bool IsGridVisible() const { return bGridVisible; }
	void DrawGrid(UCanvas* Canvas) const;

private:
	bool SampleGroundPoint(const FVector2D& GridLocation, float ReferenceZ, const struct FCollisionQueryParams& QueryParams, FVector& OutGroundPoint) const;
	void DrawSampledLine(UCanvas* Canvas, const class APlayerController& PlayerController, const struct FCollisionQueryParams& QueryParams, const FVector& Start, const FVector& End, float ReferenceZ) const;

	UPROPERTY(EditDefaultsOnly, Category = "Grid", meta = (ClampMin = "50.0"))
	float GridSize = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grid", meta = (ClampMin = "1.0"))
	float GridStep = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	FColor GridColor = FColor(0, 255, 255);

	UPROPERTY(EditDefaultsOnly, Category = "Grid", meta = (ClampMin = "0.1"))
	float LineThickness = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grid", meta = (ClampMin = "10.0"))
	float HeightSampleStep = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grid", meta = (ClampMin = "100.0"))
	float GroundTraceHalfHeight = 5000.0f;

	UPROPERTY()
	bool bGridVisible = false;
};
