#include "DdCombatComponent.h"

UDdCombatComponent::UDdCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDdCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}
