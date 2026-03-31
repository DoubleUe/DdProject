#include "DdGameLevelGameMode.h"

#include "../PlayerController/DdGameplayPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr TCHAR AllowJoinOptionKey[] = TEXT("AllowClientJoin");
}

ADdGameLevelGameMode::ADdGameLevelGameMode()
{
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
