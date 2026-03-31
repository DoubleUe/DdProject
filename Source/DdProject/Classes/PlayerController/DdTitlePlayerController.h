#pragma once

#include "GameFramework/PlayerController.h"
#include "DdTitlePlayerController.generated.h"

UCLASS()
class DDPROJECT_API ADdTitlePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	enum class ETitleSessionAction : uint8
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
	void OpenSinglePlayerLevel();
	void StartSessionAction(ETitleSessionAction InAction);
	void ExecutePendingSessionAction();
	void HandleSessionRequestFailed(const FString& ErrorMessage);
	void SetTitleScreenInteractivity(bool bIsEnabled);
	class UDdSessionSubsystem* GetSessionSubsystem() const;

	UPROPERTY()
	class UDdTitleScreenWidget* TitleScreenWidget;

	UPROPERTY()
	class UDdScreenFadeWidget* ScreenFadeWidget;

	bool bIsTransitioning = false;
	bool bExecuteSessionActionWhenFadeCompletes = false;
	ETitleSessionAction PendingSessionAction = ETitleSessionAction::None;
};
