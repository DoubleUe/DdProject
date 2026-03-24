#include "GameplayPlayerController.h"

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

AGameplayPlayerController::AGameplayPlayerController()
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

void AGameplayPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnsureScreenFadeWidget();
	EnsureResultPopupWidget();

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (ScreenFadeWidget != nullptr && Settings != nullptr)
	{
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 0.0f, Settings->FadeInDuration);
	}

	ConfigureGameplayInput();
}

void AGameplayPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent != nullptr)
	{
		InputComponent->BindKey(EKeys::LeftBracket, IE_Pressed, this, &AGameplayPlayerController::ToggleResultPopup);
		InputComponent->BindKey(EKeys::LeftAlt, IE_Pressed, this, &AGameplayPlayerController::BeginTemporaryCursorMode);
		InputComponent->BindKey(EKeys::LeftAlt, IE_Released, this, &AGameplayPlayerController::EndTemporaryCursorMode);
		InputComponent->BindKey(EKeys::RightAlt, IE_Pressed, this, &AGameplayPlayerController::BeginTemporaryCursorMode);
		InputComponent->BindKey(EKeys::RightAlt, IE_Released, this, &AGameplayPlayerController::EndTemporaryCursorMode);
	}
}

void AGameplayPlayerController::ConfigureGameplayInput()
{
	bTemporaryCursorModeActive = false;
	RefreshInputMode();
	RegisterGameplayMappingContexts();
}

void AGameplayPlayerController::RefreshInputMode()
{
	if (IsResultPopupOpen())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(ResultPopupWidget->TakeWidget());
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

void AGameplayPlayerController::RegisterGameplayMappingContexts()
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

void AGameplayPlayerController::EnsureScreenFadeWidget()
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

void AGameplayPlayerController::EnsureResultPopupWidget()
{
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
	ResultPopupWidget->OnClosed.AddUObject(this, &AGameplayPlayerController::HandleResultPopupClosed);
	ResultPopupWidget->AddToViewport(200);
	ResultPopupWidget->HidePopup();
}

void AGameplayPlayerController::ToggleResultPopup()
{
	EnsureResultPopupWidget();
	if (ResultPopupWidget == nullptr)
	{
		return;
	}

	const bool bIsVisible = ResultPopupWidget->GetVisibility() == ESlateVisibility::Visible;
	if (bIsVisible)
	{
		CloseResultPopup();
		return;
	}

	ResultPopupWidget->ShowPopup();
	RefreshInputMode();
}

void AGameplayPlayerController::CloseResultPopup()
{
	if (ResultPopupWidget == nullptr)
	{
		return;
	}

	ResultPopupWidget->HidePopup();
	HandleResultPopupClosed();
}

void AGameplayPlayerController::HandleResultPopupClosed()
{
	RefreshInputMode();
}

void AGameplayPlayerController::BeginTemporaryCursorMode()
{
	if (bTemporaryCursorModeActive)
	{
		return;
	}

	bTemporaryCursorModeActive = true;
	RefreshInputMode();
}

void AGameplayPlayerController::EndTemporaryCursorMode()
{
	if (!bTemporaryCursorModeActive)
	{
		return;
	}

	bTemporaryCursorModeActive = false;
	RefreshInputMode();
}

bool AGameplayPlayerController::IsResultPopupOpen() const
{
	return ResultPopupWidget != nullptr
		&& ResultPopupWidget->GetVisibility() == ESlateVisibility::Visible;
}
