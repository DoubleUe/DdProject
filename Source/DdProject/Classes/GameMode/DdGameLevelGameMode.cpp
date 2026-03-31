#include "DdGameLevelGameMode.h"

#include "../GameState/DdGameLevelGameState.h"
#include "../PlayerController/DdGameplayPlayerController.h"
#include "../PlayerState/DdGameplayPlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr TCHAR AllowJoinOptionKey[] = TEXT("AllowClientJoin");
}

ADdGameLevelGameMode::ADdGameLevelGameMode()
{
	GameStateClass = ADdGameLevelGameState::StaticClass();
	PlayerStateClass = ADdGameplayPlayerState::StaticClass();
	PlayerControllerClass = ADdGameplayPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> PlayerBPClass(TEXT("/Game/Characters/Player/BP_PlayerCharacter"));
	if (PlayerBPClass.Succeeded())
	{
		DefaultPawnClass = PlayerBPClass.Class;
	}
}

void ADdGameLevelGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	bAllowClientJoin = !UGameplayStatics::HasOption(Options, AllowJoinOptionKey)
		|| UGameplayStatics::GetIntOption(Options, AllowJoinOptionKey, 1) != 0;
}

void ADdGameLevelGameMode::InitGameState()
{
	Super::InitGameState();

	SyncClientJoinStateToGameState();
}

void ADdGameLevelGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	SyncClientJoinStateToGameState();

	if (NewPlayer == nullptr)
	{
		return;
	}

	if (ADdGameplayPlayerState* GameplayPlayerState = NewPlayer->GetPlayerState<ADdGameplayPlayerState>())
	{
		GameplayPlayerState->SetIsHostPlayer(NewPlayer->IsLocalController());
	}
}

void ADdGameLevelGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (!ErrorMessage.IsEmpty())
	{
		return;
	}

	if (!bAllowClientJoin)
	{
		ErrorMessage = TEXT("This single-player listen server is not accepting client joins.");
	}
}

ADdGameLevelGameState* ADdGameLevelGameMode::GetDdGameState() const
{
	return GetGameState<ADdGameLevelGameState>();
}

void ADdGameLevelGameMode::SyncClientJoinStateToGameState() const
{
	if (ADdGameLevelGameState* GameLevelGameState = GetDdGameState())
	{
		GameLevelGameState->SetAllowClientJoin(bAllowClientJoin);
	}
}
