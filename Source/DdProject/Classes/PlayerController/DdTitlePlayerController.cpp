#include "DdTitlePlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/Title/DdScreenFadeWidget.h"
#include "../UI/Title/DdTitleScreenSettings.h"
#include "../UI/Title/DdTitleScreenWidget.h"

namespace
{
	constexpr TCHAR ListenOption[] = TEXT("listen");
	constexpr TCHAR AllowJoinOption[] = TEXT("AllowClientJoin=");
	constexpr TCHAR LocalHostAddress[] = TEXT("127.0.0.1");
}

void ADdTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnsureScreenFadeWidget();

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (ScreenFadeWidget != nullptr && Settings != nullptr)
	{
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 0.0f, Settings->FadeInDuration);
	}

	ShowTitleScreen();
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
	if (!bExecuteTitleActionWhenFadeCompletes)
	{
		return;
	}

	bExecuteTitleActionWhenFadeCompletes = false;
	GetWorldTimerManager().SetTimerForNextTick(this, &ADdTitlePlayerController::ExecutePendingTitleAction);
}

void ADdTitlePlayerController::EnterSingleGame()
{
	StartTitleAction(ETitleAction::Single);
}

void ADdTitlePlayerController::EnterHostGame()
{
	StartTitleAction(ETitleAction::Host);
}

void ADdTitlePlayerController::EnterJoinGame()
{
	StartTitleAction(ETitleAction::Join);
}

void ADdTitlePlayerController::StartTitleAction(ETitleAction InAction)
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

	bIsTransitioning = true;
	PendingTitleAction = InAction;
	SetTitleScreenInteractivity(false);
	EnsureScreenFadeWidget();

	const float FadeOutDuration = FMath::Max(Settings->FadeOutDuration, 0.0f);
	if (ScreenFadeWidget != nullptr && FadeOutDuration > KINDA_SMALL_NUMBER)
	{
		bExecuteTitleActionWhenFadeCompletes = true;
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 1.0f, FadeOutDuration);
		return;
	}

	ExecutePendingTitleAction();
}

void ADdTitlePlayerController::ExecutePendingTitleAction()
{
	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr)
	{
		HandleTitleActionFailed(TEXT("Title settings are unavailable."));
		return;
	}

	switch (PendingTitleAction)
	{
	case ETitleAction::Single:
		OpenHostedLevel(false);
		return;
	case ETitleAction::Host:
		OpenHostedLevel(true);
		return;
	case ETitleAction::Join:
		JoinLocalHost();
		return;
	default:
		break;
	}

	HandleTitleActionFailed(TEXT("The requested title action is invalid."));
}

void ADdTitlePlayerController::OpenHostedLevel(const bool bAllowClientJoin)
{
	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr || Settings->GameLevelName.IsNone())
	{
		HandleTitleActionFailed(TEXT("Gameplay level travel target is invalid."));
		return;
	}

	PendingTitleAction = ETitleAction::None;

	const FString TravelOptions = FString::Printf(
		TEXT("%s?%s%s"),
		ListenOption,
		AllowJoinOption,
		bAllowClientJoin ? TEXT("1") : TEXT("0"));

	UGameplayStatics::OpenLevel(this, Settings->GameLevelName, true, TravelOptions);
}

void ADdTitlePlayerController::JoinLocalHost()
{
	PendingTitleAction = ETitleAction::None;
	ClientTravel(LocalHostAddress, TRAVEL_Absolute);
}

void ADdTitlePlayerController::HandleTitleActionFailed(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Title action failed: %s"), *ErrorMessage);

	bIsTransitioning = false;
	bExecuteTitleActionWhenFadeCompletes = false;
	PendingTitleAction = ETitleAction::None;
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
