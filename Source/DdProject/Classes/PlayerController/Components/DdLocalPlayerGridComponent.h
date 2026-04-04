#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DdLocalPlayerGridComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DDPROJECT_API UDdLocalPlayerGridComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDdLocalPlayerGridComponent();

	void SetGridVisible(bool bVisible);
	bool IsGridVisible() const { return bGridVisible; }

protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void DrawLocalGrid(const FVector& Center) const;

	UPROPERTY(EditDefaultsOnly, Category = "Grid", meta = (ClampMin = "50.0"))
	float GridSize = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grid", meta = (ClampMin = "1.0"))
	float GridStep = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grid")
	FColor GridColor = FColor(0, 255, 255);

	UPROPERTY(EditDefaultsOnly, Category = "Grid", meta = (ClampMin = "0.1"))
	float LineThickness = 1.0f;

	UPROPERTY()
	bool bGridVisible = false;
};
