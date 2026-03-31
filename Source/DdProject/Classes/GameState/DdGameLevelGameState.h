#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DdGameLevelGameState.generated.h"

class FLifetimeProperty;

UCLASS(BlueprintType, Blueprintable)
class DDPROJECT_API ADdGameLevelGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ADdGameLevelGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Network")
	bool AllowsClientJoin() const;

	void SetAllowClientJoin(bool bInAllowClientJoin);

protected:
	UPROPERTY(Replicated, VisibleInstanceOnly, BlueprintReadOnly, Category = "Network")
	bool bAllowClientJoin = true;
};
