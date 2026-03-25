#pragma once

#include "GameFramework/PlayerController.h"
#include "DdTitlePlayerController.generated.h"

UCLASS()
class DDPROJECT_API ADdTitlePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	void ConfigureTitleInput();
	void EnsureScreenFadeWidget();
	void HandleScreenFadeFinished();
	void ShowTitleScreen();
	void EnterGame();
	void OpenPendingLevel();

	UPROPERTY()
	class UDdTitleScreenWidget* TitleScreenWidget;

	UPROPERTY()
	class UDdScreenFadeWidget* ScreenFadeWidget;

	FName PendingLevelName;

	bool bIsTransitioning = false;
	bool bOpenLevelWhenFadeCompletes = false;
};
