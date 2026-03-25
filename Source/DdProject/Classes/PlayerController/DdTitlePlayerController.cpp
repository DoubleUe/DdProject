#include "DdTitlePlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/Title/DdScreenFadeWidget.h"
#include "../UI/Title/DdTitleScreenSettings.h"
#include "../UI/Title/DdTitleScreenWidget.h"

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

	TitleScreenWidget->OnStartGameRequested.AddUObject(this, &ADdTitlePlayerController::EnterGame);
	TitleScreenWidget->AddToViewport(100);
	ConfigureTitleInput();
}

void ADdTitlePlayerController::HandleScreenFadeFinished()
{
	if (!bOpenLevelWhenFadeCompletes)
	{
		return;
	}

	bOpenLevelWhenFadeCompletes = false;
	GetWorldTimerManager().SetTimerForNextTick(this, &ADdTitlePlayerController::OpenPendingLevel);
}

void ADdTitlePlayerController::EnterGame()
{
	if (bIsTransitioning)
	{
		return;
	}

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
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

void ADdTitlePlayerController::OpenPendingLevel()
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
