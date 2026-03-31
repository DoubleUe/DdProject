#include "DdGameLevelGameState.h"

#include "Net/UnrealNetwork.h"

ADdGameLevelGameState::ADdGameLevelGameState()
{
}

void ADdGameLevelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADdGameLevelGameState, bAllowClientJoin);
}

bool ADdGameLevelGameState::AllowsClientJoin() const
{
	return bAllowClientJoin;
}

void ADdGameLevelGameState::SetAllowClientJoin(bool bInAllowClientJoin)
{
	bAllowClientJoin = bInAllowClientJoin;
}
