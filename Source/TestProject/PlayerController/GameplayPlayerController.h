#pragma once

#include "GameFramework/PlayerController.h"
#include "GameplayPlayerController.generated.h"

UCLASS()
class TESTPROJECT_API AGameplayPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGameplayPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	void ConfigureGameplayInput();
	void RegisterGameplayMappingContexts();
	void EnsureScreenFadeWidget();
	void EnsureResultPopupWidget();
	void ToggleResultPopup();
	void CloseResultPopup();
	void HandleResultPopupClosed();

	UPROPERTY()
	class UScreenFadeWidget* ScreenFadeWidget;

	UPROPERTY()
	class UResultPopupWidget* ResultPopupWidget;

	UPROPERTY()
	class UInputMappingContext* GameplayDefaultMappingContext;

	UPROPERTY()
	class UInputMappingContext* GameplayMouseLookMappingContext;
};
