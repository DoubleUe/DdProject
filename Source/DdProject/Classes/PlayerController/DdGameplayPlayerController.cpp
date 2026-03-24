#include "DdGameplayPlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "../UI/Gameplay/DdResultPopupSettings.h"
#include "../UI/Gameplay/DdResultPopupWidget.h"
#include "../UI/Title/DdScreenFadeWidget.h"
#include "../UI/Title/DdTitleScreenSettings.h"

ADdGameplayPlayerController::ADdGameplayPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextAsset(TEXT("/Game/Characters/Player/Input/IMC_Default.IMC_Default"));
	if (DefaultMappingContextAsset.Succeeded())
	{
		GameplayDefaultMappingContext = DefaultMappingContextAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookMappingContextAsset(TEXT("/Game/Characters/Player/Input/IMC_MouseLook.IMC_MouseLook"));
	if (MouseLookMappingContextAsset.Succeeded())
	{
		GameplayMouseLookMappingContext = MouseLookMappingContextAsset.Object;
	}
}

void ADdGameplayPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnsureDdScreenFadeWidget();
	EnsureDdResultPopupWidget();

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (DdScreenFadeWidget != nullptr && Settings != nullptr)
	{
		DdScreenFadeWidget->StartFade(DdScreenFadeWidget->GetFadeAlpha(), 0.0f, Settings->FadeInDuration);
	}

	ConfigureGameplayInput();
}

void ADdGameplayPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent != nullptr)
	{
		InputComponent->BindKey(EKeys::LeftBracket, IE_Pressed, this, &ADdGameplayPlayerController::ToggleResultPopup);
		InputComponent->BindKey(EKeys::LeftAlt, IE_Pressed, this, &ADdGameplayPlayerController::BeginTemporaryCursorMode);
		InputComponent->BindKey(EKeys::LeftAlt, IE_Released, this, &ADdGameplayPlayerController::EndTemporaryCursorMode);
		InputComponent->BindKey(EKeys::RightAlt, IE_Pressed, this, &ADdGameplayPlayerController::BeginTemporaryCursorMode);
		InputComponent->BindKey(EKeys::RightAlt, IE_Released, this, &ADdGameplayPlayerController::EndTemporaryCursorMode);
	}
}

void ADdGameplayPlayerController::ConfigureGameplayInput()
{
	bTemporaryCursorModeActive = false;
	RefreshInputMode();
	RegisterGameplayMappingContexts();
}

void ADdGameplayPlayerController::RefreshInputMode()
{
	if (IsResultPopupOpen())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(DdResultPopupWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);

		bShowMouseCursor = true;
		bEnableClickEvents = true;
		bEnableMouseOverEvents = true;
		return;
	}

	if (bTemporaryCursorModeActive)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);

		bShowMouseCursor = true;
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;
		return;
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}

void ADdGameplayPlayerController::RegisterGameplayMappingContexts()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (InputSubsystem == nullptr)
	{
		return;
	}

	if (GameplayDefaultMappingContext != nullptr)
	{
		InputSubsystem->RemoveMappingContext(GameplayDefaultMappingContext);
		InputSubsystem->AddMappingContext(GameplayDefaultMappingContext, 0);
	}

	if (GameplayMouseLookMappingContext != nullptr)
	{
		InputSubsystem->RemoveMappingContext(GameplayMouseLookMappingContext);
		InputSubsystem->AddMappingContext(GameplayMouseLookMappingContext, 0);
	}
}

void ADdGameplayPlayerController::EnsureDdScreenFadeWidget()
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

	if (DdScreenFadeWidget != nullptr)
	{
		return;
	}

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

void ADdGameplayPlayerController::EnsureDdResultPopupWidget()
{
	if (DdResultPopupWidget != nullptr)
	{
		return;
	}

	const UDdResultPopupSettings* Settings = GetDefault<UDdResultPopupSettings>();
	UClass* DdResultPopupWidgetClass = Settings != nullptr ? Settings->DdResultPopupWidgetClass.LoadSynchronous() : nullptr;
	if (DdResultPopupWidgetClass == nullptr)
	{
		DdResultPopupWidgetClass = UDdResultPopupWidget::StaticClass();
	}

	DdResultPopupWidget = CreateWidget<UDdResultPopupWidget>(this, DdResultPopupWidgetClass);
	if (DdResultPopupWidget == nullptr)
	{
		return;
	}

	DdResultPopupWidget->OnClosed.RemoveAll(this);
	DdResultPopupWidget->OnClosed.AddUObject(this, &ADdGameplayPlayerController::HandleResultPopupClosed);
	DdResultPopupWidget->AddToViewport(200);
	DdResultPopupWidget->HidePopup();
}

void ADdGameplayPlayerController::ToggleResultPopup()
{
	EnsureDdResultPopupWidget();
	if (DdResultPopupWidget == nullptr)
	{
		return;
	}

	const bool bIsVisible = DdResultPopupWidget->GetVisibility() == ESlateVisibility::Visible;
	if (bIsVisible)
	{
		CloseResultPopup();
		return;
	}

	DdResultPopupWidget->ShowPopup();
	RefreshInputMode();
}

void ADdGameplayPlayerController::CloseResultPopup()
{
	if (DdResultPopupWidget == nullptr)
	{
		return;
	}

	DdResultPopupWidget->HidePopup();
	HandleResultPopupClosed();
}

void ADdGameplayPlayerController::HandleResultPopupClosed()
{
	RefreshInputMode();
}

void ADdGameplayPlayerController::BeginTemporaryCursorMode()
{
	if (bTemporaryCursorModeActive)
	{
		return;
	}

	bTemporaryCursorModeActive = true;
	RefreshInputMode();
}

void ADdGameplayPlayerController::EndTemporaryCursorMode()
{
	if (!bTemporaryCursorModeActive)
	{
		return;
	}

	bTemporaryCursorModeActive = false;
	RefreshInputMode();
}

bool ADdGameplayPlayerController::IsResultPopupOpen() const
{
	return DdResultPopupWidget != nullptr
		&& DdResultPopupWidget->GetVisibility() == ESlateVisibility::Visible;
}
