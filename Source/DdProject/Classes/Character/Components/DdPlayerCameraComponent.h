#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DdPlayerCameraComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DDPROJECT_API UDdPlayerCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDdPlayerCameraComponent();

	void ZoomCamera(float Value);
	void UpdateCameraZoom(float DeltaSeconds);

protected:
	virtual void OnRegister() override;

private:
	void CreateCameraComponents();
	void RefreshCameraZoomBounds();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float CameraZoomStep = 50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera", meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float CameraZoomInterpSpeed = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float MinCameraBoomLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float MaxCameraBoomLength = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	float DesiredCameraBoomLength = 0.0f;

	UPROPERTY()
	bool bCameraZoomBoundsDirty = true;

	UPROPERTY()
	float CachedCapsuleZoomLimit = -1.0f;
};
