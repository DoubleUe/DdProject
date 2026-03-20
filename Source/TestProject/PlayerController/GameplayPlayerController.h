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

private:
	void ConfigureGameplayInput();
	void RegisterGameplayMappingContexts();
	void EnsureScreenFadeWidget();

	UPROPERTY()
	class UScreenFadeWidget* ScreenFadeWidget;

	UPROPERTY()
	class UInputMappingContext* GameplayDefaultMappingContext;

	UPROPERTY()
	class UInputMappingContext* GameplayMouseLookMappingContext;
};
