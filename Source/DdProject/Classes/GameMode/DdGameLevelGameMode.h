#pragma once

#include "GameFramework/GameModeBase.h"
#include "DdGameLevelGameMode.generated.h"

UCLASS(Blueprintable)
class DDPROJECT_API ADdGameLevelGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADdGameLevelGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

private:
	bool bAllowClientJoin = true;
};
