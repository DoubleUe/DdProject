#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerCameraComponent.generated.h"

class UCameraComponent;
class USpringArmComponent;

// 플레이어 캐릭터의 카메라 붐 및 카메라를 관리하는 컴포넌트
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DDPROJECT_API UPlayerCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerCameraComponent();

	// 줌 입력 처리
	void ZoomCamera(float Value);

	// 매 프레임 줌 보간 업데이트
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
