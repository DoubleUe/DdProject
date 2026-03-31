#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DdGameplayPlayerState.generated.h"

class FLifetimeProperty;

UCLASS(BlueprintType, Blueprintable)
class DDPROJECT_API ADdGameplayPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ADdGameplayPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Network")
	bool IsHostPlayer() const;

	void SetIsHostPlayer(bool bInIsHostPlayer);

protected:
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Network")
	bool bIsHostPlayer = false;
};
