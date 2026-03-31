#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "DdSessionSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDdOnSessionRequestFailed, const FString&);

UENUM()
enum class EDdSessionRequestType : uint8
{
	None,
	Host,
	Join
};

UCLASS()
class DDPROJECT_API UDdSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	FDdOnSessionRequestFailed OnSessionRequestFailed;

	bool HostSession(const FName& InLevelName);
	bool JoinFirstAvailableSession();
	bool IsRequestInProgress() const;

private:
	static const FName SessionName;
	static constexpr int32 MaxPublicConnections = 4;
	static constexpr int32 MaxSearchResults = 32;

	bool StartFindSessions(EDdSessionRequestType InRequestType);
	bool StartCreateSession(EDdSessionRequestType InRequestType, const FName& InLevelName);
	bool CreateSessionInternal();
	bool JoinSessionInternal(const FOnlineSessionSearchResult& SearchResult);
	bool DestroyExistingSessionIfNeeded();
	void ClearDelegateHandles();
	void ResetRequestState();
	void BroadcastRequestFailed(const FString& ErrorMessage);
	IOnlineSessionPtr GetSessionInterface() const;
	const FOnlineSessionSearchResult* GetFirstValidSearchResult() const;

	void HandleCreateSessionComplete(FName InSessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName InSessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName InSessionName, bool bWasSuccessful);

	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	FDelegateHandle CreateSessionCompleteHandle;
	FDelegateHandle FindSessionsCompleteHandle;
	FDelegateHandle JoinSessionCompleteHandle;
	FDelegateHandle DestroySessionCompleteHandle;

	FName PendingLevelName;
	EDdSessionRequestType PendingRequestType = EDdSessionRequestType::None;
	bool bRecreateSessionAfterDestroy = false;
};
