#include "DdGameplayPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "../Character/DdBaseCharacter.h"
#include "../Character/DdPlayerCharacter.h"
#include "../UI/Gameplay/DdResultPopupSettings.h"
#include "../UI/Gameplay/DdResultPopupWidget.h"
#include "../UI/Title/DdScreenFadeWidget.h"
#include "../UI/Title/DdTitleScreenSettings.h"

ADdGameplayPlayerController::ADdGameplayPlayerController()
{
	GameplayUtilityMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("GameplayUtilityMappingContext"));
	GameplayToggleResultPopupAction = CreateDefaultSubobject<UInputAction>(TEXT("GameplayToggleResultPopupAction"));
	GameplayTemporaryCursorAction = CreateDefaultSubobject<UInputAction>(TEXT("GameplayTemporaryCursorAction"));
	GameplayAttackAction = CreateDefaultSubobject<UInputAction>(TEXT("GameplayAttackAction"));
	GameplayCameraZoomAction = CreateDefaultSubobject<UInputAction>(TEXT("GameplayCameraZoomAction"));
	GameplayToggleRotationModeAction = CreateDefaultSubobject<UInputAction>(TEXT("GameplayToggleRotationModeAction"));
	GameplayToggleWalkSpeedAction = CreateDefaultSubobject<UInputAction>(TEXT("GameplayToggleWalkSpeedAction"));

	static ConstructorHelpers::FObjectFinder<UInputAction> JumpActionAsset(TEXT("/Game/Design/Input/Actions/IA_Jump.IA_Jump"));
	if (JumpActionAsset.Succeeded())
	{
		GameplayJumpAction = JumpActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> MoveActionAsset(TEXT("/Game/Design/Input/Actions/IA_Move.IA_Move"));
	if (MoveActionAsset.Succeeded())
	{
		GameplayMoveAction = MoveActionAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputAction> LookActionAsset(TEXT("/Game/Design/Input/Actions/IA_Look.IA_Look"));
	if (LookActionAsset.Succeeded())
	{
		GameplayLookAction = LookActionAsset.Object;
	}

	if (GameplayToggleResultPopupAction != nullptr)
	{
		GameplayToggleResultPopupAction->ValueType = EInputActionValueType::Boolean;
	}

	if (GameplayTemporaryCursorAction != nullptr)
	{
		GameplayTemporaryCursorAction->ValueType = EInputActionValueType::Boolean;
	}

	if (GameplayAttackAction != nullptr)
	{
		GameplayAttackAction->ValueType = EInputActionValueType::Boolean;
	}

	if (GameplayCameraZoomAction != nullptr)
	{
		GameplayCameraZoomAction->ValueType = EInputActionValueType::Axis1D;
	}

	if (GameplayToggleRotationModeAction != nullptr)
	{
		GameplayToggleRotationModeAction->ValueType = EInputActionValueType::Boolean;
	}

	if (GameplayToggleWalkSpeedAction != nullptr)
	{
		GameplayToggleWalkSpeedAction->ValueType = EInputActionValueType::Boolean;
	}

	if (GameplayUtilityMappingContext != nullptr)
	{
		if (GameplayAttackAction != nullptr)
		{
			GameplayUtilityMappingContext->MapKey(GameplayAttackAction, EKeys::LeftMouseButton);
		}

		if (GameplayCameraZoomAction != nullptr)
		{
			GameplayUtilityMappingContext->MapKey(GameplayCameraZoomAction, EKeys::MouseWheelAxis);
		}

		if (GameplayToggleRotationModeAction != nullptr)
		{
			GameplayUtilityMappingContext->MapKey(GameplayToggleRotationModeAction, EKeys::R);
		}

		if (GameplayToggleWalkSpeedAction != nullptr)
		{
			GameplayUtilityMappingContext->MapKey(GameplayToggleWalkSpeedAction, EKeys::LeftControl);
		}

		if (GameplayToggleResultPopupAction != nullptr)
		{
			GameplayUtilityMappingContext->MapKey(GameplayToggleResultPopupAction, EKeys::LeftBracket);
		}

		if (GameplayTemporaryCursorAction != nullptr)
		{
			GameplayUtilityMappingContext->MapKey(GameplayTemporaryCursorAction, EKeys::LeftAlt);
			GameplayUtilityMappingContext->MapKey(GameplayTemporaryCursorAction, EKeys::RightAlt);
		}
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextAsset(TEXT("/Game/Design/Input/IMC_Default.IMC_Default"));
	if (DefaultMappingContextAsset.Succeeded())
	{
		GameplayDefaultMappingContext = DefaultMappingContextAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookMappingContextAsset(TEXT("/Game/Design/Input/IMC_MouseLook.IMC_MouseLook"));
	if (MouseLookMappingContextAsset.Succeeded())
	{
		GameplayMouseLookMappingContext = MouseLookMappingContextAsset.Object;
	}
}

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

void ADdGameplayPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (!IsLocalController())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	if (EnhancedInputComponent == nullptr)
	{
		return;
	}

	BindGameplayInputActions(EnhancedInputComponent);
}

void ADdGameplayPlayerController::ConfigureGameplayInput()
{
	bTemporaryCursorModeActive = false;
	RefreshInputMode();
	RegisterGameplayMappingContexts();
}

void ADdGameplayPlayerController::BindGameplayInputActions(UEnhancedInputComponent* EnhancedInputComponent)
{
	if (EnhancedInputComponent == nullptr)
	{
		return;
	}

	if (GameplayJumpAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayJumpAction, ETriggerEvent::Started, this, &ADdGameplayPlayerController::HandleJumpStarted);
		EnhancedInputComponent->BindAction(GameplayJumpAction, ETriggerEvent::Completed, this, &ADdGameplayPlayerController::HandleJumpCompleted);
	}

	if (GameplayMoveAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayMoveAction, ETriggerEvent::Triggered, this, &ADdGameplayPlayerController::HandleMoveTriggered);
	}

	if (GameplayLookAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayLookAction, ETriggerEvent::Triggered, this, &ADdGameplayPlayerController::HandleLookTriggered);
	}

	if (GameplayAttackAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayAttackAction, ETriggerEvent::Started, this, &ADdGameplayPlayerController::HandleAttackStarted);
	}

	if (GameplayCameraZoomAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayCameraZoomAction, ETriggerEvent::Triggered, this, &ADdGameplayPlayerController::HandleCameraZoomTriggered);
	}

	if (GameplayToggleRotationModeAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayToggleRotationModeAction, ETriggerEvent::Started, this, &ADdGameplayPlayerController::HandleToggleRotationModeStarted);
	}

	if (GameplayToggleWalkSpeedAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayToggleWalkSpeedAction, ETriggerEvent::Started, this, &ADdGameplayPlayerController::HandleToggleWalkSpeedStarted);
	}

	if (GameplayToggleResultPopupAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayToggleResultPopupAction, ETriggerEvent::Started, this, &ADdGameplayPlayerController::ToggleResultPopup);
	}

	if (GameplayTemporaryCursorAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayTemporaryCursorAction, ETriggerEvent::Started, this, &ADdGameplayPlayerController::BeginTemporaryCursorMode);
		EnhancedInputComponent->BindAction(GameplayTemporaryCursorAction, ETriggerEvent::Completed, this, &ADdGameplayPlayerController::EndTemporaryCursorMode);
	}
}

void ADdGameplayPlayerController::HandleJumpStarted()
{
	if (ADdPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter())
	{
		PlayerCharacter->Jump();
	}
}

void ADdGameplayPlayerController::HandleJumpCompleted()
{
	if (ADdPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter())
	{
		PlayerCharacter->StopJumping();
	}
}

void ADdGameplayPlayerController::HandleMoveTriggered(const FInputActionValue& Value)
{
	if (ADdPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter())
	{
		PlayerCharacter->ApplyMoveInput(Value);
	}
}

void ADdGameplayPlayerController::HandleLookTriggered(const FInputActionValue& Value)
{
	if (ADdPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter())
	{
		PlayerCharacter->ApplyLookInput(Value);
	}
}

void ADdGameplayPlayerController::HandleAttackStarted()
{
	if (ADdPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter())
	{
		PlayerCharacter->TryAttack();
	}
}

void ADdGameplayPlayerController::HandleCameraZoomTriggered(const FInputActionValue& Value)
{
	if (ADdPlayerCharacter* PlayerCharacter = GetControlledPlayerCharacter())
	{
		PlayerCharacter->ApplyCameraZoomInput(Value);
	}
}

void ADdGameplayPlayerController::HandleToggleRotationModeStarted()
{
	if (ADdBaseCharacter* BaseCharacter = GetControlledBaseCharacter())
	{
		BaseCharacter->ToggleRotationMode();
	}
}

void ADdGameplayPlayerController::HandleToggleWalkSpeedStarted()
{
	if (ADdBaseCharacter* BaseCharacter = GetControlledBaseCharacter())
	{
		BaseCharacter->ToggleWalkSpeed();
	}
}

void ADdGameplayPlayerController::RefreshInputMode()
{
	if (!IsLocalController())
	{
		return;
	}

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

	if (GameplayUtilityMappingContext != nullptr)
	{
		InputSubsystem->RemoveMappingContext(GameplayUtilityMappingContext);
		InputSubsystem->AddMappingContext(GameplayUtilityMappingContext, 1);
	}
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

void ADdGameplayPlayerController::ToggleResultPopup()
{
	if (!IsLocalController())
	{
		return;
	}

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

void ADdGameplayPlayerController::HandleResultPopupClosed()
{
	RefreshInputMode();
}

void ADdGameplayPlayerController::BeginTemporaryCursorMode()
{
	if (!IsLocalController())
	{
		return;
	}

	if (bTemporaryCursorModeActive)
	{
		return;
	}

	bTemporaryCursorModeActive = true;
	RefreshInputMode();
}

void ADdGameplayPlayerController::EndTemporaryCursorMode()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!bTemporaryCursorModeActive)
	{
		return;
	}

	bTemporaryCursorModeActive = false;
	RefreshInputMode();
}

ADdPlayerCharacter* ADdGameplayPlayerController::GetControlledPlayerCharacter() const
{
	return Cast<ADdPlayerCharacter>(GetPawn());
}

ADdBaseCharacter* ADdGameplayPlayerController::GetControlledBaseCharacter() const
{
	return Cast<ADdBaseCharacter>(GetPawn());
}

bool ADdGameplayPlayerController::IsResultPopupOpen() const
{
	return ResultPopupWidget != nullptr
		&& ResultPopupWidget->GetVisibility() == ESlateVisibility::Visible;
}
