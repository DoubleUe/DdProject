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
#include "../UI/Gameplay/DdResultPopupWidget.h"

ADdGameplayPlayerController::ADdGameplayPlayerController()
{
	GameplayUtilityMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("GameplayUtilityMappingContext"));
	GameplayToggleResultPopupAction = CreateDefaultSubobject<UInputAction>(TEXT("GameplayToggleResultPopupAction"));
	GameplayFreeCursorAction = CreateDefaultSubobject<UInputAction>(TEXT("GameplayFreeCursorAction"));
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

	if (GameplayFreeCursorAction != nullptr)
	{
		GameplayFreeCursorAction->ValueType = EInputActionValueType::Boolean;
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

		if (GameplayFreeCursorAction != nullptr)
		{
			GameplayUtilityMappingContext->MapKey(GameplayFreeCursorAction, EKeys::LeftAlt);
			GameplayUtilityMappingContext->MapKey(GameplayFreeCursorAction, EKeys::RightAlt);
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
	bFreeCursorModeActive = false;
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

	if (GameplayFreeCursorAction != nullptr)
	{
		EnhancedInputComponent->BindAction(GameplayFreeCursorAction, ETriggerEvent::Started, this, &ADdGameplayPlayerController::BeginFreeCursorMode);
		EnhancedInputComponent->BindAction(GameplayFreeCursorAction, ETriggerEvent::Completed, this, &ADdGameplayPlayerController::EndFreeCursorMode);
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

	if (bFreeCursorModeActive)
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

void ADdGameplayPlayerController::HandleResultPopupClosed()
{
	RefreshInputMode();
}

void ADdGameplayPlayerController::BeginFreeCursorMode()
{
	if (!IsLocalController())
	{
		return;
	}

	if (bFreeCursorModeActive)
	{
		return;
	}

	bFreeCursorModeActive = true;
	RefreshInputMode();
}

void ADdGameplayPlayerController::EndFreeCursorMode()
{
	if (!IsLocalController())
	{
		return;
	}

	if (!bFreeCursorModeActive)
	{
		return;
	}

	bFreeCursorModeActive = false;
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
