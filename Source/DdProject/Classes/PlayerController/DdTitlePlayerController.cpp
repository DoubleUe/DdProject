#include "DdTitlePlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../UI/Title/DdScreenFadeWidget.h"
#include "../UI/Title/DdTitleScreenSettings.h"
#include "../UI/Title/DdTitleScreenWidget.h"

void ADdTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnsureDdScreenFadeWidget();

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (DdScreenFadeWidget != nullptr && Settings != nullptr)
	{
		DdScreenFadeWidget->StartFade(DdScreenFadeWidget->GetFadeAlpha(), 0.0f, Settings->FadeInDuration);
	}

	ShowTitleScreen();
}

void ADdTitlePlayerController::ConfigureTitleInput()
{
	FInputModeUIOnly InputMode;
	if (DdTitleScreenWidget != nullptr)
	{
		InputMode.SetWidgetToFocus(DdTitleScreenWidget->TakeWidget());
	}
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ADdTitlePlayerController::EnsureDdScreenFadeWidget()
{
	if (DdScreenFadeWidget == nullptr)
	{
		TArray<UUserWidget*> FoundWidgets;
		UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, FoundWidgets, UDdScreenFadeWidget::StaticClass(), false);

		for (UUserWidget* FoundWidget : FoundWidgets)
		{
			if (UDdScreenFadeWidget* ExistingFadeWidget = Cast<UDdScreenFadeWidget>(FoundWidget))
			{
				DdScreenFadeWidget = ExistingFadeWidget;
				break;
			}
		}
	}

	if (DdScreenFadeWidget == nullptr)
	{
		const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
		UClass* DdScreenFadeWidgetClass = Settings != nullptr ? Settings->DdScreenFadeWidgetClass.LoadSynchronous() : nullptr;
		if (DdScreenFadeWidgetClass == nullptr)
		{
			DdScreenFadeWidgetClass = UDdScreenFadeWidget::StaticClass();
		}

		DdScreenFadeWidget = CreateWidget<UDdScreenFadeWidget>(GetGameInstance(), DdScreenFadeWidgetClass);
		if (DdScreenFadeWidget != nullptr)
		{
			DdScreenFadeWidget->AddToViewport(1000);
			DdScreenFadeWidget->SetFadeAlpha(1.0f);
		}
	}

	if (DdScreenFadeWidget != nullptr)
	{
		DdScreenFadeWidget->OnFadeFinished.RemoveAll(this);
		DdScreenFadeWidget->OnFadeFinished.AddUObject(this, &ADdTitlePlayerController::HandleScreenFadeFinished);
	}
}

void ADdTitlePlayerController::ShowTitleScreen()
{
	if (!IsLocalController() || DdTitleScreenWidget != nullptr)
	{
		return;
	}

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr)
	{
		return;
	}

	UClass* TitleWidgetClass = Settings->DdTitleScreenWidgetClass.LoadSynchronous();
	if (TitleWidgetClass == nullptr)
	{
		TitleWidgetClass = UDdTitleScreenWidget::StaticClass();
	}

	DdTitleScreenWidget = CreateWidget<UDdTitleScreenWidget>(this, TitleWidgetClass);
	if (DdTitleScreenWidget == nullptr)
	{
		return;
	}

	DdTitleScreenWidget->OnStartGameRequested.AddUObject(this, &ADdTitlePlayerController::EnterGame);
	DdTitleScreenWidget->AddToViewport(100);
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

	if (DdTitleScreenWidget != nullptr)
	{
		DdTitleScreenWidget->SetIsEnabled(false);
		DdTitleScreenWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	EnsureDdScreenFadeWidget();

	const float FadeOutDuration = FMath::Max(Settings->FadeOutDuration, 0.0f);
	if (DdScreenFadeWidget != nullptr && FadeOutDuration > KINDA_SMALL_NUMBER)
	{
		bOpenLevelWhenFadeCompletes = true;
		DdScreenFadeWidget->StartFade(DdScreenFadeWidget->GetFadeAlpha(), 1.0f, FadeOutDuration);
		return;
	}

	OpenPendingLevel();
}

void ADdTitlePlayerController::OpenPendingLevel()
{
	if (DdTitleScreenWidget != nullptr)
	{
		DdTitleScreenWidget->RemoveFromParent();
		DdTitleScreenWidget = nullptr;
	}

	if (PendingLevelName.IsNone())
	{
		bIsTransitioning = false;
		return;
	}

	UGameplayStatics::OpenLevel(this, PendingLevelName);
}
