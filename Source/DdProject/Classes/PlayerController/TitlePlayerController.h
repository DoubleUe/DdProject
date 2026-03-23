#pragma once

#include "GameFramework/PlayerController.h"
#include "TitlePlayerController.generated.h"

UCLASS()
class DDPROJECT_API ATitlePlayerController : public APlayerController
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
	class UTitleScreenWidget* TitleScreenWidget;

	UPROPERTY()
	class UScreenFadeWidget* ScreenFadeWidget;

	FName PendingLevelName;

	bool bIsTransitioning = false;
	bool bOpenLevelWhenFadeCompletes = false;
};
