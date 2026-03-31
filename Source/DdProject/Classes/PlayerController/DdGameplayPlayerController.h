#pragma once

#include "GameFramework/PlayerController.h"
#include "DdGameplayPlayerController.generated.h"

UCLASS()
class DDPROJECT_API ADdGameplayPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADdGameplayPlayerController();
	const class UInputAction* GetGameplayAttackAction() const { return GameplayAttackAction; }
	const class UInputAction* GetGameplayCameraZoomAction() const { return GameplayCameraZoomAction; }
	const class UInputAction* GetGameplayToggleRotationModeAction() const { return GameplayToggleRotationModeAction; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void ConfigureGameplayInput();
	void RefreshInputMode();
	void RegisterGameplayMappingContexts();
	void EnsureScreenFadeWidget();
	void EnsureResultPopupWidget();
	void ToggleResultPopup();
	void CloseResultPopup();
	void HandleResultPopupClosed();
	void BeginTemporaryCursorMode();
	void EndTemporaryCursorMode();
	bool IsResultPopupOpen() const;

	UPROPERTY()
	class UDdScreenFadeWidget* ScreenFadeWidget;

	UPROPERTY()
	class UDdResultPopupWidget* ResultPopupWidget;

	UPROPERTY()
	class UInputMappingContext* GameplayDefaultMappingContext;

	UPROPERTY()
	class UInputMappingContext* GameplayMouseLookMappingContext;

	UPROPERTY()
	class UInputMappingContext* GameplayUtilityMappingContext;

	UPROPERTY()
	class UInputAction* GameplayAttackAction;

	UPROPERTY()
	class UInputAction* GameplayCameraZoomAction;

	UPROPERTY()
	class UInputAction* GameplayToggleRotationModeAction;

	UPROPERTY()
	bool bTemporaryCursorModeActive = false;
};
