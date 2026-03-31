#include "DdSessionSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystem.h"

namespace
{
	constexpr TCHAR SessionTravelSuffix[] = TEXT("?listen");
}

const FName UDdSessionSubsystem::SessionName(TEXT("GameSession"));

void UDdSessionSubsystem::Deinitialize()
{
	ClearDelegateHandles();
	ResetRequestState();

	Super::Deinitialize();
}

bool UDdSessionSubsystem::HostSession(const FName& InLevelName)
{
	return StartCreateSession(EDdSessionRequestType::Host, InLevelName);
}

bool UDdSessionSubsystem::JoinFirstAvailableSession()
{
	return StartFindSessions(EDdSessionRequestType::Join);
}

bool UDdSessionSubsystem::IsRequestInProgress() const
{
	return PendingRequestType != EDdSessionRequestType::None;
}

bool UDdSessionSubsystem::StartFindSessions(EDdSessionRequestType InRequestType)
{
	if (IsRequestInProgress())
	{
		return false;
	}

	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	PendingRequestType = InRequestType;

	SessionSearch = MakeShared<FOnlineSessionSearch>();
	SessionSearch->bIsLanQuery = true;
	SessionSearch->MaxSearchResults = MaxSearchResults;

	FindSessionsCompleteHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(this, &UDdSessionSubsystem::HandleFindSessionsComplete));

	if (SessionInterface->FindSessions(0, SessionSearch.ToSharedRef()))
	{
		return true;
	}

	ClearDelegateHandles();
	ResetRequestState();
	return false;
}

bool UDdSessionSubsystem::StartCreateSession(EDdSessionRequestType InRequestType, const FName& InLevelName)
{
	if (IsRequestInProgress())
	{
		return false;
	}

	if (InLevelName.IsNone())
	{
		return false;
	}

	PendingRequestType = InRequestType;
	PendingLevelName = InLevelName;
	return DestroyExistingSessionIfNeeded();
}

bool UDdSessionSubsystem::CreateSessionInternal()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	LastSessionSettings = MakeShared<FOnlineSessionSettings>();
	LastSessionSettings->bIsLANMatch = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->NumPublicConnections = MaxPublicConnections;
	LastSessionSettings->NumPrivateConnections = 0;
	LastSessionSettings->BuildUniqueId = 1;

	CreateSessionCompleteHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(this, &UDdSessionSubsystem::HandleCreateSessionComplete));

	if (SessionInterface->CreateSession(0, SessionName, *LastSessionSettings))
	{
		return true;
	}

	ClearDelegateHandles();
	return false;
}

bool UDdSessionSubsystem::JoinSessionInternal(const FOnlineSessionSearchResult& SearchResult)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	JoinSessionCompleteHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(this, &UDdSessionSubsystem::HandleJoinSessionComplete));

	if (SessionInterface->JoinSession(0, SessionName, SearchResult))
	{
		return true;
	}

	ClearDelegateHandles();
	return false;
}

bool UDdSessionSubsystem::DestroyExistingSessionIfNeeded()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		return false;
	}

	if (SessionInterface->GetNamedSession(SessionName) == nullptr)
	{
		return CreateSessionInternal();
	}

	bRecreateSessionAfterDestroy = true;
	DestroySessionCompleteHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(this, &UDdSessionSubsystem::HandleDestroySessionComplete));

	if (SessionInterface->DestroySession(SessionName))
	{
		return true;
	}

	ClearDelegateHandles();
	bRecreateSessionAfterDestroy = false;
	return false;
}

void UDdSessionSubsystem::ClearDelegateHandles()
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		CreateSessionCompleteHandle.Reset();
		FindSessionsCompleteHandle.Reset();
		JoinSessionCompleteHandle.Reset();
		DestroySessionCompleteHandle.Reset();
		return;
	}

	if (CreateSessionCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
		CreateSessionCompleteHandle.Reset();
	}

	if (FindSessionsCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
		FindSessionsCompleteHandle.Reset();
	}

	if (JoinSessionCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
		JoinSessionCompleteHandle.Reset();
	}

	if (DestroySessionCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
		DestroySessionCompleteHandle.Reset();
	}
}

void UDdSessionSubsystem::ResetRequestState()
{
	LastSessionSettings.Reset();
	SessionSearch.Reset();
	PendingLevelName = NAME_None;
	PendingRequestType = EDdSessionRequestType::None;
	bRecreateSessionAfterDestroy = false;
}

void UDdSessionSubsystem::BroadcastRequestFailed(const FString& ErrorMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Session request failed: %s"), *ErrorMessage);

	ClearDelegateHandles();
	ResetRequestState();
	OnSessionRequestFailed.Broadcast(ErrorMessage);
}

IOnlineSessionPtr UDdSessionSubsystem::GetSessionInterface() const
{
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	return OnlineSubsystem != nullptr ? OnlineSubsystem->GetSessionInterface() : nullptr;
}

const FOnlineSessionSearchResult* UDdSessionSubsystem::GetFirstValidSearchResult() const
{
	if (!SessionSearch.IsValid())
	{
		return nullptr;
	}

	for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
	{
		if (SearchResult.IsValid())
		{
			return &SearchResult;
		}
	}

	return nullptr;
}

void UDdSessionSubsystem::HandleCreateSessionComplete(FName InSessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid() && CreateSessionCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteHandle);
		CreateSessionCompleteHandle.Reset();
	}

	if (!bWasSuccessful)
	{
		BroadcastRequestFailed(TEXT("Failed to create a LAN session."));
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr || PendingLevelName.IsNone())
	{
		BroadcastRequestFailed(TEXT("Session created but travel target is invalid."));
		return;
	}

	const FString TravelURL = PendingLevelName.ToString() + SessionTravelSuffix;
	ResetRequestState();
	World->ServerTravel(TravelURL);
}

void UDdSessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid() && FindSessionsCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteHandle);
		FindSessionsCompleteHandle.Reset();
	}

	if (!bWasSuccessful)
	{
		BroadcastRequestFailed(TEXT("Failed to search LAN sessions."));
		return;
	}

	const FOnlineSessionSearchResult* SearchResult = GetFirstValidSearchResult();
	if (SearchResult != nullptr)
	{
		if (!JoinSessionInternal(*SearchResult))
		{
			BroadcastRequestFailed(TEXT("Found a LAN session but failed to join it."));
		}
		return;
	}

	BroadcastRequestFailed(TEXT("No LAN session was found."));
}

void UDdSessionSubsystem::HandleJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid() && JoinSessionCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteHandle);
		JoinSessionCompleteHandle.Reset();
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		BroadcastRequestFailed(TEXT("Failed to join the selected LAN session."));
		return;
	}

	FString ConnectString;
	if (!SessionInterface.IsValid() || !SessionInterface->GetResolvedConnectString(InSessionName, ConnectString))
	{
		BroadcastRequestFailed(TEXT("Joined the session but failed to resolve the host address."));
		return;
	}

	UWorld* World = GetWorld();
	APlayerController* PlayerController = World != nullptr ? World->GetFirstPlayerController() : nullptr;
	if (PlayerController == nullptr)
	{
		BroadcastRequestFailed(TEXT("Joined the session but the local player controller is invalid."));
		return;
	}

	ResetRequestState();
	PlayerController->ClientTravel(ConnectString, TRAVEL_Absolute);
}

void UDdSessionSubsystem::HandleDestroySessionComplete(FName InSessionName, bool bWasSuccessful)
{
	IOnlineSessionPtr SessionInterface = GetSessionInterface();
	if (SessionInterface.IsValid() && DestroySessionCompleteHandle.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteHandle);
		DestroySessionCompleteHandle.Reset();
	}

	if (!bWasSuccessful)
	{
		BroadcastRequestFailed(TEXT("Failed to destroy the previous session."));
		return;
	}

	if (!bRecreateSessionAfterDestroy)
	{
		ResetRequestState();
		return;
	}

	bRecreateSessionAfterDestroy = false;
	if (!CreateSessionInternal())
	{
		BroadcastRequestFailed(TEXT("Destroyed the previous session but failed to create a new LAN session."));
	}
}
