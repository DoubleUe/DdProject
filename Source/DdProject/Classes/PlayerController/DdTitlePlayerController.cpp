#include "DdTitlePlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../Network/DdSessionSubsystem.h"
#include "../UI/Title/DdScreenFadeWidget.h"
#include "../UI/Title/DdTitleScreenSettings.h"
#include "../UI/Title/DdTitleScreenWidget.h"

void ADdTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UDdSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnSessionRequestFailed.RemoveAll(this);
		SessionSubsystem->OnSessionRequestFailed.AddUObject(this, &ADdTitlePlayerController::HandleSessionRequestFailed);
	}

	EnsureScreenFadeWidget();

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (ScreenFadeWidget != nullptr && Settings != nullptr)
	{
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 0.0f, Settings->FadeInDuration);
	}

	ShowTitleScreen();
}

void ADdTitlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDdSessionSubsystem* SessionSubsystem = GetSessionSubsystem())
	{
		SessionSubsystem->OnSessionRequestFailed.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void ADdTitlePlayerController::ConfigureTitleInput()
{
	FInputModeUIOnly InputMode;
	if (TitleScreenWidget != nullptr)
	{
		InputMode.SetWidgetToFocus(TitleScreenWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ADdTitlePlayerController::EnsureScreenFadeWidget()
{
	if (ScreenFadeWidget == nullptr)
	{
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, UDdScreenFadeWidget::StaticClass(), false);

		for (UUserWidget* FoundWidget : FoundWidgets)
		{
			if (UDdScreenFadeWidget* ExistingFadeWidget = Cast<UDdScreenFadeWidget>(FoundWidget))
			{
				ScreenFadeWidget = ExistingFadeWidget;
				break;
			}
		}
	}

	if (ScreenFadeWidget == nullptr)
	{
		const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
		UClass* ScreenFadeWidgetClass = Settings != nullptr ? Settings->ScreenFadeWidgetClass.LoadSynchronous() : nullptr;
		if (ScreenFadeWidgetClass == nullptr)
		{
			ScreenFadeWidgetClass = UDdScreenFadeWidget::StaticClass();
		}

		ScreenFadeWidget = CreateWidget<UDdScreenFadeWidget>(GetGameInstance(), ScreenFadeWidgetClass);
		if (ScreenFadeWidget != nullptr)
		{
			ScreenFadeWidget->AddToViewport(1000);
			ScreenFadeWidget->SetFadeAlpha(1.0f);
		}
	}

	if (ScreenFadeWidget != nullptr)
	{
		ScreenFadeWidget->OnFadeFinished.RemoveAll(this);
		ScreenFadeWidget->OnFadeFinished.AddUObject(this, &ADdTitlePlayerController::HandleScreenFadeFinished);
	}
}

void ADdTitlePlayerController::ShowTitleScreen()
{
	if (!IsLocalController() || TitleScreenWidget != nullptr)
	{
		return;
	}

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr)
	{
		return;
	}

	UClass* TitleWidgetClass = Settings->TitleScreenWidgetClass.LoadSynchronous();
	if (TitleWidgetClass == nullptr)
	{
		TitleWidgetClass = UDdTitleScreenWidget::StaticClass();
	}

	TitleScreenWidget = CreateWidget<UDdTitleScreenWidget>(this, TitleWidgetClass);
	if (TitleScreenWidget == nullptr)
	{
		return;
	}

	TitleScreenWidget->OnSingleGameRequested.AddUObject(this, &ADdTitlePlayerController::EnterSingleGame);
	TitleScreenWidget->OnHostGameRequested.AddUObject(this, &ADdTitlePlayerController::EnterHostGame);
	TitleScreenWidget->OnJoinGameRequested.AddUObject(this, &ADdTitlePlayerController::EnterJoinGame);
	TitleScreenWidget->AddToViewport(100);
	ConfigureTitleInput();
}

void ADdTitlePlayerController::HandleScreenFadeFinished()
{
	if (!bExecuteSessionActionWhenFadeCompletes)
	{
		return;
	}

	bExecuteSessionActionWhenFadeCompletes = false;
	GetWorldTimerManager().SetTimerForNextTick(this, &ADdTitlePlayerController::ExecutePendingSessionAction);
}

void ADdTitlePlayerController::EnterSingleGame()
{
	StartSessionAction(ETitleSessionAction::Single);
}

void ADdTitlePlayerController::EnterHostGame()
{
	StartSessionAction(ETitleSessionAction::Host);
}

void ADdTitlePlayerController::EnterJoinGame()
{
	StartSessionAction(ETitleSessionAction::Join);
}

void ADdTitlePlayerController::StartSessionAction(ETitleSessionAction InAction)
{
	if (bIsTransitioning)
	{
		return;
	}

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr || GetWorld() == nullptr)
	{
		return;
	}

	if (InAction != ETitleSessionAction::Single && GetSessionSubsystem() == nullptr)
	{
		return;
	}

	bIsTransitioning = true;
	PendingSessionAction = InAction;
	SetTitleScreenInteractivity(false);
	EnsureScreenFadeWidget();

	const float FadeOutDuration = FMath::Max(Settings->FadeOutDuration, 0.0f);
	if (ScreenFadeWidget != nullptr && FadeOutDuration > KINDA_SMALL_NUMBER)
	{
		bExecuteSessionActionWhenFadeCompletes = true;
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 1.0f, FadeOutDuration);
		return;
	}

	ExecutePendingSessionAction();
}

void ADdTitlePlayerController::ExecutePendingSessionAction()
{
	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr)
	{
		HandleSessionRequestFailed(TEXT("Session subsystem is unavailable."));
		return;
	}

	switch (PendingSessionAction)
	{
	case ETitleSessionAction::Single:
		OpenSinglePlayerLevel();
		return;
	default:
		break;
	}

	UDdSessionSubsystem* SessionSubsystem = GetSessionSubsystem();
	if (SessionSubsystem == nullptr)
	{
		HandleSessionRequestFailed(TEXT("Session subsystem is unavailable."));
		return;
	}

	bool bRequestStarted = false;
	switch (PendingSessionAction)
	{
	case ETitleSessionAction::Host:
		bRequestStarted = SessionSubsystem->HostSession(Settings->GameLevelName);
		break;
	case ETitleSessionAction::Join:
		bRequestStarted = SessionSubsystem->JoinFirstAvailableSession();
		break;
	}

	if (!bRequestStarted)
	{
		HandleSessionRequestFailed(TEXT("Failed to start the session request."));
	}
}

void ADdTitlePlayerController::OpenSinglePlayerLevel()
{
	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr || Settings->GameLevelName.IsNone())
	{
		HandleSessionRequestFailed(TEXT("Single-player travel target is invalid."));
		return;
	}

	PendingSessionAction = ETitleSessionAction::None;
	UGameplayStatics::OpenLevel(this, Settings->GameLevelName);
}

void ADdTitlePlayerController::HandleSessionRequestFailed(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Title session request failed: %s"), *ErrorMessage);

	bIsTransitioning = false;
	bExecuteSessionActionWhenFadeCompletes = false;
	PendingSessionAction = ETitleSessionAction::None;
	SetTitleScreenInteractivity(true);

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	const float FadeInDuration = Settings != nullptr ? FMath::Max(Settings->FadeInDuration, 0.0f) : 0.0f;

	if (ScreenFadeWidget != nullptr)
	{
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 0.0f, FadeInDuration);
	}
}

void ADdTitlePlayerController::SetTitleScreenInteractivity(bool bIsEnabled)
{
	if (TitleScreenWidget != nullptr)
	{
		TitleScreenWidget->SetIsEnabled(bIsEnabled);
		TitleScreenWidget->SetVisibility(bIsEnabled ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
	}

	if (bIsEnabled)
	{
		ConfigureTitleInput();
		return;
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

UDdSessionSubsystem* ADdTitlePlayerController::GetSessionSubsystem() const
{
	UGameInstance* GameInstance = GetGameInstance();
	return GameInstance != nullptr ? GameInstance->GetSubsystem<UDdSessionSubsystem>() : nullptr;
}
