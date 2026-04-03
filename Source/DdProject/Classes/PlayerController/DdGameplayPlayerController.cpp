#include "DdGameplayPlayerController.h"

#include "../UI/Gameplay/DdResultPopupSettings.h"
#include "../UI/Gameplay/DdResultPopupWidget.h"
#include "../UI/Title/DdScreenFadeWidget.h"
#include "../UI/Title/DdTitleScreenSettings.h"

void ADdGameplayPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController())
	{
		return;
	}

	InitializeSharedLocalControllerState();
	EnsureResultPopupWidget();

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (ScreenFadeWidget != nullptr && Settings != nullptr)
	{
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 0.0f, Settings->FadeInDuration);
	}

	ConfigureGameplayInput();
}

void ADdGameplayPlayerController::EnsureResultPopupWidget()
{
	if (!IsLocalController())
	{
		return;
	}

	if (ResultPopupWidget != nullptr)
	{
		return;
	}

	const UDdResultPopupSettings* Settings = GetDefault<UDdResultPopupSettings>();
	UClass* ResultPopupWidgetClass = Settings != nullptr ? Settings->ResultPopupWidgetClass.LoadSynchronous() : nullptr;
	if (ResultPopupWidgetClass == nullptr)
	{
		ResultPopupWidgetClass = UDdResultPopupWidget::StaticClass();
	}

	ResultPopupWidget = CreateWidget<UDdResultPopupWidget>(this, ResultPopupWidgetClass);
	if (ResultPopupWidget == nullptr)
	{
		return;
	}

	ResultPopupWidget->OnClosed.RemoveAll(this);
	ResultPopupWidget->OnClosed.AddUObject(this, &ADdGameplayPlayerController::HandleResultPopupClosed);
	ResultPopupWidget->AddToViewport(200);
	ResultPopupWidget->HidePopup();
}

void ADdGameplayPlayerController::CloseResultPopup()
{
	if (!IsLocalController())
	{
		return;
	}

	if (ResultPopupWidget == nullptr)
	{
		return;
	}

	ResultPopupWidget->HidePopup();
	HandleResultPopupClosed();
}
