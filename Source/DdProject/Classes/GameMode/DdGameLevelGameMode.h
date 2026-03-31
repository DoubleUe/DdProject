#pragma once

#include "GameFramework/GameModeBase.h"
#include "DdGameLevelGameMode.generated.h"

class ADdGameLevelGameState;
class APlayerController;

UCLASS(Blueprintable)
class DDPROJECT_API ADdGameLevelGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADdGameLevelGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

private:
	ADdGameLevelGameState* GetDdGameState() const;
	void SyncClientJoinStateToGameState() const;

	bool bAllowClientJoin = true;
};
