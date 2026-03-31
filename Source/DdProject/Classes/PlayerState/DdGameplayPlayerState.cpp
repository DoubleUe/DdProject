#include "DdGameplayPlayerState.h"

#include "Net/UnrealNetwork.h"

ADdGameplayPlayerState::ADdGameplayPlayerState()
{
}

void ADdGameplayPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADdGameplayPlayerState, bIsHostPlayer);
}

bool ADdGameplayPlayerState::IsHostPlayer() const
{
	return bIsHostPlayer;
}

void ADdGameplayPlayerState::SetIsHostPlayer(bool bInIsHostPlayer)
{
	bIsHostPlayer = bInIsHostPlayer;
}
