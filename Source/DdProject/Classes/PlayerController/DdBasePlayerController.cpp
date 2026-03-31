#include "DdBasePlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "../UI/Title/DdScreenFadeWidget.h"
#include "../UI/Title/DdTitleScreenSettings.h"

void ADdBasePlayerController::InitializeSharedLocalControllerState()
{
	if (!IsLocalController())
	{
		return;
	}

	ApplyDefaultViewportStats();
	EnsureScreenFadeWidget();
}

void ADdBasePlayerController::ApplyDefaultViewportStats()
{
	if (!IsLocalController() || GEngine == nullptr)
	{
		return;
	}

	UWorld* const World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	if (World->GetGameViewport() == nullptr)
	{
		World->GetTimerManager().SetTimerForNextTick(this, &ADdBasePlayerController::ApplyDefaultViewportStats);
		return;
	}

	GEngine->SetEngineStat(World, World->GetGameViewport(), TEXT("FPS"), true);
	GEngine->SetEngineStat(World, World->GetGameViewport(), TEXT("Unit"), true);
}

void ADdBasePlayerController::EnsureScreenFadeWidget()
{
	if (!IsLocalController())
	{
		return;
	}

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

	if (ScreenFadeWidget != nullptr)
	{
		return;
	}

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
