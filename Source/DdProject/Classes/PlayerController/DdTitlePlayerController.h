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
	enum class ETitleAction : uint8
	{
		None,
		Single,
		Host,
		Join
	};

	void ConfigureTitleInput();
	void EnsureScreenFadeWidget();
	void HandleScreenFadeFinished();
	void ShowTitleScreen();
	void EnterSingleGame();
	void EnterHostGame();
	void EnterJoinGame();
	void OpenHostedLevel(bool bAllowClientJoin);
	void JoinLocalHost();
	void StartTitleAction(ETitleAction InAction);
	void ExecutePendingTitleAction();
	void HandleTitleActionFailed(const FString& ErrorMessage);
	void SetTitleScreenInteractivity(bool bIsEnabled);

	UPROPERTY()
	class UDdTitleScreenWidget* TitleScreenWidget;

	UPROPERTY()
	class UDdScreenFadeWidget* ScreenFadeWidget;

	bool bIsTransitioning = false;
	bool bExecuteTitleActionWhenFadeCompletes = false;
	ETitleAction PendingTitleAction = ETitleAction::None;
};
