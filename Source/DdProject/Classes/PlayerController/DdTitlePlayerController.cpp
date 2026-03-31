#include "DdTitlePlayerController.h"

#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "TimerManager.h"
#include "../UI/Title/DdScreenFadeWidget.h"
#include "../UI/Title/DdTitleScreenSettings.h"
#include "../UI/Title/DdTitleScreenWidget.h"

namespace
{
	constexpr TCHAR ListenOption[] = TEXT("listen");
	constexpr TCHAR AllowJoinOption[] = TEXT("AllowClientJoin=");
	constexpr TCHAR LocalHostAddress[] = TEXT("127.0.0.1");
	constexpr int32 DefaultListenPort = 7777;
}

void ADdTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine != nullptr)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
		GEngine->OnNetworkFailure().AddUObject(this, &ADdTitlePlayerController::HandleNetworkFailure);
	}

	InitializeSharedLocalControllerState();
	BindScreenFadeWidgetEvents();

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (ScreenFadeWidget != nullptr && Settings != nullptr)
	{
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 0.0f, Settings->FadeInDuration);
	}

	ShowTitleScreen();
}

void ADdTitlePlayerController::BindScreenFadeWidgetEvents()
{
	if (ScreenFadeWidget == nullptr)
	{
		return;
	}

	ScreenFadeWidget->OnFadeFinished.RemoveAll(this);
	ScreenFadeWidget->OnFadeFinished.AddUObject(this, &ADdTitlePlayerController::HandleScreenFadeFinished);
}

void ADdTitlePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GEngine != nullptr)
	{
		GEngine->OnNetworkFailure().RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
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

	TitleScreenWidget->OnSingleGameRequested.AddUObject(this, &ADdTitlePlayerController::EnterSingleGame);
	TitleScreenWidget->OnHostGameRequested.AddUObject(this, &ADdTitlePlayerController::EnterHostGame);
	TitleScreenWidget->OnJoinGameRequested.AddUObject(this, &ADdTitlePlayerController::EnterJoinGame);
	TitleScreenWidget->AddToViewport(100);
	ConfigureTitleInput();
}

void ADdTitlePlayerController::HandleScreenFadeFinished()
{
	if (!bExecuteTitleActionWhenFadeCompletes)
	{
		return;
	}

	bExecuteTitleActionWhenFadeCompletes = false;
	GetWorldTimerManager().SetTimerForNextTick(this, &ADdTitlePlayerController::ExecutePendingTitleAction);
}

void ADdTitlePlayerController::EnterSingleGame()
{
	StartTitleAction(ETitleAction::Single);
}

void ADdTitlePlayerController::EnterHostGame()
{
	StartTitleAction(ETitleAction::Host);
}

void ADdTitlePlayerController::EnterJoinGame()
{
	StartTitleAction(ETitleAction::Join);
}

void ADdTitlePlayerController::StartTitleAction(ETitleAction InAction)
{
	if (bIsTransitioning)
	{
		return;
	}

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr || GetWorld() == nullptr)
	{
		return;
	}

	FString ErrorMessage;
	if (!CanStartTitleAction(InAction, ErrorMessage))
	{
		HandleTitleActionFailed(ErrorMessage);
		return;
	}

	bIsTransitioning = true;
	PendingTitleAction = InAction;
	SetTitleScreenInteractivity(false);
	EnsureScreenFadeWidget();
	BindScreenFadeWidgetEvents();

	const float FadeOutDuration = FMath::Max(Settings->FadeOutDuration, 0.0f);
	if (ScreenFadeWidget != nullptr && FadeOutDuration > KINDA_SMALL_NUMBER)
	{
		bExecuteTitleActionWhenFadeCompletes = true;
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 1.0f, FadeOutDuration);
		return;
	}

	ExecutePendingTitleAction();
}

void ADdTitlePlayerController::ExecutePendingTitleAction()
{
	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr)
	{
		HandleTitleActionFailed(TEXT("Title settings are unavailable."));
		return;
	}

	switch (PendingTitleAction)
	{
	case ETitleAction::Single:
		OpenHostedLevel(false);
		return;
	case ETitleAction::Host:
		OpenHostedLevel(true);
		return;
	case ETitleAction::Join:
		JoinLocalHost();
		return;
	default:
		break;
	}

	HandleTitleActionFailed(TEXT("The requested title action is invalid."));
}

void ADdTitlePlayerController::OpenHostedLevel(const bool bAllowClientJoin)
{
	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	if (Settings == nullptr || Settings->GameLevelName.IsNone())
	{
		HandleTitleActionFailed(TEXT("Gameplay level travel target is invalid."));
		return;
	}

	PendingTitleAction = ETitleAction::None;

	const FString TravelOptions = FString::Printf(
		TEXT("%s?%s%s"),
		ListenOption,
		AllowJoinOption,
		bAllowClientJoin ? TEXT("1") : TEXT("0"));

	UGameplayStatics::OpenLevel(this, Settings->GameLevelName, true, TravelOptions);
}

void ADdTitlePlayerController::JoinLocalHost()
{
	PendingTitleAction = ETitleAction::None;
	ClientTravel(LocalHostAddress, TRAVEL_Absolute);
}

bool ADdTitlePlayerController::CanStartTitleAction(const ETitleAction InAction, FString& OutErrorMessage) const
{
	switch (InAction)
	{
	case ETitleAction::Single:
	case ETitleAction::Host:
		if (!IsLocalHostPortAvailable(DefaultListenPort))
		{
			OutErrorMessage = FString::Printf(TEXT("Port %d is already in use."), DefaultListenPort);
			return false;
		}
		return true;
	case ETitleAction::Join:
		if (IsLocalHostPortAvailable(DefaultListenPort))
		{
			OutErrorMessage = FString::Printf(TEXT("No host is listening on %s:%d."), LocalHostAddress, DefaultListenPort);
			return false;
		}
		return true;
	default:
		return true;
	}
}

bool ADdTitlePlayerController::IsLocalHostPortAvailable(const int32 InPort) const
{
	ISocketSubsystem* const SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (SocketSubsystem == nullptr)
	{
		return false;
	}

	FSocket* const TestSocket = SocketSubsystem->CreateSocket(NAME_DGram, TEXT("DdTitlePortAvailabilityCheck"), false);
	if (TestSocket == nullptr)
	{
		return false;
	}

	TSharedRef<FInternetAddr> BindAddress = SocketSubsystem->CreateInternetAddr();
	BindAddress->SetAnyAddress();
	BindAddress->SetPort(InPort);

	const bool bWasAvailable = TestSocket->Bind(*BindAddress);

	TestSocket->Close();
	SocketSubsystem->DestroySocket(TestSocket);
	return bWasAvailable;
}

void ADdTitlePlayerController::ShowTitleErrorMessage(const FString& ErrorMessage) const
{
	UKismetSystemLibrary::PrintString(this, ErrorMessage, true, true, FLinearColor::Red, 5.0f);
}

void ADdTitlePlayerController::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, const ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	if (!IsLocalController())
	{
		return;
	}

	FString FailureMessage = ErrorString;
	if (FailureMessage.IsEmpty())
	{
		switch (FailureType)
		{
		case ENetworkFailure::ConnectionLost:
			FailureMessage = TEXT("Connection lost.");
			break;
		case ENetworkFailure::ConnectionTimeout:
			FailureMessage = TEXT("Connection timed out.");
			break;
		case ENetworkFailure::FailureReceived:
			FailureMessage = TEXT("A network failure was received from the host.");
			break;
		case ENetworkFailure::PendingConnectionFailure:
			FailureMessage = TEXT("Failed to connect to the host.");
			break;
		case ENetworkFailure::NetGuidMismatch:
			FailureMessage = TEXT("Network GUID mismatch.");
			break;
		case ENetworkFailure::NetChecksumMismatch:
			FailureMessage = TEXT("Network checksum mismatch.");
			break;
		default:
			FailureMessage = TEXT("A network failure occurred.");
			break;
		}
	}

	HandleTitleActionFailed(FailureMessage);
}

void ADdTitlePlayerController::HandleTitleActionFailed(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Title action failed: %s"), *ErrorMessage);
	ShowTitleErrorMessage(ErrorMessage);

	bIsTransitioning = false;
	bExecuteTitleActionWhenFadeCompletes = false;
	PendingTitleAction = ETitleAction::None;
	SetTitleScreenInteractivity(true);

	const UDdTitleScreenSettings* Settings = GetDefault<UDdTitleScreenSettings>();
	const float FadeInDuration = Settings != nullptr ? FMath::Max(Settings->FadeInDuration, 0.0f) : 0.0f;

	if (ScreenFadeWidget != nullptr)
	{
		ScreenFadeWidget->StartFade(ScreenFadeWidget->GetFadeAlpha(), 0.0f, FadeInDuration);
	}
}

void ADdTitlePlayerController::SetTitleScreenInteractivity(bool bIsEnabled)
{
	if (TitleScreenWidget != nullptr)
	{
		TitleScreenWidget->SetIsEnabled(bIsEnabled);
		TitleScreenWidget->SetVisibility(bIsEnabled ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
	}

	if (bIsEnabled)
	{
		ConfigureTitleInput();
		return;
	}

	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
}
