#pragma once

#include "DdBasePlayerController.h"
#include "DdGameplayPlayerController.generated.h"

struct FInputActionValue;

UCLASS()
class DDPROJECT_API ADdGameplayPlayerController : public ADdBasePlayerController
{
	GENERATED_BODY()

public:
	ADdGameplayPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void ConfigureGameplayInput();
	void RefreshInputMode();
	void RegisterGameplayMappingContexts();
	void BindGameplayInputActions(class UEnhancedInputComponent* EnhancedInputComponent);
	void EnsureResultPopupWidget();
	void HandleJumpStarted();
	void HandleJumpCompleted();
	void HandleMoveTriggered(const FInputActionValue& Value);
	void HandleLookTriggered(const FInputActionValue& Value);
	void HandleAttackStarted();
	void HandleCameraZoomTriggered(const FInputActionValue& Value);
	void HandleToggleRotationModeStarted();
	void HandleToggleWalkSpeedStarted();
	void ToggleResultPopup();
	void CloseResultPopup();
	void HandleResultPopupClosed();
	void BeginTemporaryCursorMode();
	void EndTemporaryCursorMode();
	class ADdPlayerCharacter* GetControlledPlayerCharacter() const;
	class ADdBaseCharacter* GetControlledBaseCharacter() const;
	bool IsResultPopupOpen() const;

	UPROPERTY()
	class UDdResultPopupWidget* ResultPopupWidget;

	UPROPERTY()
	class UInputMappingContext* GameplayDefaultMappingContext;

	UPROPERTY()
	class UInputMappingContext* GameplayMouseLookMappingContext;

	UPROPERTY()
	class UInputMappingContext* GameplayUtilityMappingContext;

	UPROPERTY()
	class UInputAction* GameplayJumpAction;

	UPROPERTY()
	class UInputAction* GameplayMoveAction;

	UPROPERTY()
	class UInputAction* GameplayLookAction;

	UPROPERTY()
	class UInputAction* GameplayAttackAction;

	UPROPERTY()
	class UInputAction* GameplayCameraZoomAction;

	UPROPERTY()
	class UInputAction* GameplayToggleRotationModeAction;

	UPROPERTY()
	class UInputAction* GameplayToggleWalkSpeedAction;

	UPROPERTY()
	class UInputAction* GameplayToggleResultPopupAction;

	UPROPERTY()
	class UInputAction* GameplayTemporaryCursorAction;

	UPROPERTY()
	bool bTemporaryCursorModeActive = false;
};
