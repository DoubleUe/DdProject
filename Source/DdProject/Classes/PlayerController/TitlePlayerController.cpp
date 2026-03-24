#include "TitlePlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/Title/ScreenFadeWidget.h"
#include "../UI/Title/TitleScreenSettings.h"
#include "../UI/Title/TitleScreenWidget.h"

void ATitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnsureScreenFadeWidget();

	const UTitleScreenSettings* Settings = GetDefault<UTitleScreenSettings>();
	if (ScreenFadeWidget != nullptr && Settings != nullptr)
	{
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 0.0f, Settings->FadeInDuration);
	}

	ShowTitleScreen();
}

void ATitlePlayerController::ConfigureTitleInput()
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

void ATitlePlayerController::EnsureScreenFadeWidget()
{
	if (ScreenFadeWidget == nullptr)
	{
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, UScreenFadeWidget::StaticClass(), false);

		for (UUserWidget* FoundWidget : FoundWidgets)
		{
			if (UScreenFadeWidget* ExistingFadeWidget = Cast<UScreenFadeWidget>(FoundWidget))
			{
				ScreenFadeWidget = ExistingFadeWidget;
				break;
			}
		}
	}

	if (ScreenFadeWidget == nullptr)
	{
		const UTitleScreenSettings* Settings = GetDefault<UTitleScreenSettings>();
		UClass* ScreenFadeWidgetClass = Settings != nullptr ? Settings->ScreenFadeWidgetClass.LoadSynchronous() : nullptr;
		if (ScreenFadeWidgetClass == nullptr)
		{
			ScreenFadeWidgetClass = UScreenFadeWidget::StaticClass();
		}

		ScreenFadeWidget = CreateWidget<UScreenFadeWidget>(GetGameInstance(), ScreenFadeWidgetClass);
		if (ScreenFadeWidget != nullptr)
		{
			ScreenFadeWidget->AddToViewport(1000);
			ScreenFadeWidget->SetFadeAlpha(1.0f);
		}
	}

	if (ScreenFadeWidget != nullptr)
	{
		ScreenFadeWidget->OnFadeFinished.RemoveAll(this);
		ScreenFadeWidget->OnFadeFinished.AddUObject(this, &ATitlePlayerController::HandleScreenFadeFinished);
	}
}

void ATitlePlayerController::ShowTitleScreen()
{
	if (!IsLocalController() || TitleScreenWidget != nullptr)
	{
		return;
	}

	const UTitleScreenSettings* Settings = GetDefault<UTitleScreenSettings>();
	if (Settings == nullptr)
	{
		return;
	}

	UClass* TitleWidgetClass = Settings->TitleScreenWidgetClass.LoadSynchronous();
	if (TitleWidgetClass == nullptr)
	{
		TitleWidgetClass = UTitleScreenWidget::StaticClass();
	}

	TitleScreenWidget = CreateWidget<UTitleScreenWidget>(this, TitleWidgetClass);
	if (TitleScreenWidget == nullptr)
	{
		return;
	}

	TitleScreenWidget->OnStartGameRequested.AddUObject(this, &ATitlePlayerController::EnterGame);
	TitleScreenWidget->AddToViewport(100);
	ConfigureTitleInput();
}

void ATitlePlayerController::HandleScreenFadeFinished()
{
	if (!bOpenLevelWhenFadeCompletes)
	{
		return;
	}

	bOpenLevelWhenFadeCompletes = false;
	GetWorldTimerManager().SetTimerForNextTick(this, &ATitlePlayerController::OpenPendingLevel);
}

void ATitlePlayerController::EnterGame()
{
	if (bIsTransitioning)
	{
		return;
	}

	const UTitleScreenSettings* Settings = GetDefault<UTitleScreenSettings>();
	if (Settings == nullptr || Settings->GameLevelName.IsNone() || GetWorld() == nullptr)
	{
		return;
	}

	bIsTransitioning = true;
	PendingLevelName = Settings->GameLevelName;

	if (TitleScreenWidget != nullptr)
	{
		TitleScreenWidget->SetIsEnabled(false);
		TitleScreenWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	EnsureScreenFadeWidget();

	const float FadeOutDuration = FMath::Max(Settings->FadeOutDuration, 0.0f);
	if (ScreenFadeWidget != nullptr && FadeOutDuration > KINDA_SMALL_NUMBER)
	{
		bOpenLevelWhenFadeCompletes = true;
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 1.0f, FadeOutDuration);
		return;
	}

	OpenPendingLevel();
}

void ATitlePlayerController::OpenPendingLevel()
{
	if (TitleScreenWidget != nullptr)
	{
		TitleScreenWidget->RemoveFromParent();
		TitleScreenWidget = nullptr;
	}

	if (PendingLevelName.IsNone())
	{
		bIsTransitioning = false;
		return;
	}

	UGameplayStatics::OpenLevel(this, PendingLevelName);
}
