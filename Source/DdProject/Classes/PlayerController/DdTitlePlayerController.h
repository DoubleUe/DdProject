#pragma once

#include "Engine/EngineBaseTypes.h"
#include "GameFramework/PlayerController.h"
#include "DdTitlePlayerController.generated.h"

class UNetDriver;

UCLASS()
class DDPROJECT_API ADdTitlePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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
	bool CanStartTitleAction(ETitleAction InAction, FString& OutErrorMessage) const;
	bool IsLocalHostPortAvailable(int32 InPort) const;
	void ShowTitleErrorMessage(const FString& ErrorMessage) const;
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
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
