#include "GameplayPlayerController.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "../UI/Gameplay/ResultPopupSettings.h"
#include "../UI/Gameplay/ResultPopupWidget.h"
#include "../UI/Title/ScreenFadeWidget.h"
#include "../UI/Title/TitleScreenSettings.h"

AGameplayPlayerController::AGameplayPlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextAsset(TEXT("/Game/Characters/ThirdPerson/Input/IMC_Default.IMC_Default"));
	if (DefaultMappingContextAsset.Succeeded())
	{
		GameplayDefaultMappingContext = DefaultMappingContextAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookMappingContextAsset(TEXT("/Game/Characters/ThirdPerson/Input/IMC_MouseLook.IMC_MouseLook"));
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

	const UTitleScreenSettings* Settings = GetDefault<UTitleScreenSettings>();
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
	}
}

void AGameplayPlayerController::ConfigureGameplayInput()
{
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	RegisterGameplayMappingContexts();
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

	if (ScreenFadeWidget != nullptr)
	{
		return;
	}

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

void AGameplayPlayerController::EnsureResultPopupWidget()
{
	if (ResultPopupWidget != nullptr)
	{
		return;
	}

	const UResultPopupSettings* Settings = GetDefault<UResultPopupSettings>();
	UClass* ResultPopupWidgetClass = Settings != nullptr ? Settings->ResultPopupWidgetClass.LoadSynchronous() : nullptr;
	if (ResultPopupWidgetClass == nullptr)
	{
		ResultPopupWidgetClass = UResultPopupWidget::StaticClass();
	}

	ResultPopupWidget = CreateWidget<UResultPopupWidget>(this, ResultPopupWidgetClass);
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

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(ResultPopupWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
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
	ConfigureGameplayInput();
}
