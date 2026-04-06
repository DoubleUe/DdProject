#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DdCombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DDPROJECT_API UDdCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDdCombatComponent();

protected:
	virtual void BeginPlay() override;
};
