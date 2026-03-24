#include "BTDecorator_IsInAttackRange.h"

#include "AIController.h"
#include "Character/DdMonsterCharacter.h"

UBTDecorator_IsInAttackRange::UBTDecorator_IsInAttackRange()
{
	NodeName = TEXT("공격 범위 확인");
}

bool UBTDecorator_IsInAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController == nullptr)
	{
		return false;
	}

	ADdMonsterCharacter* Monster = Cast<ADdMonsterCharacter>(AIController->GetPawn());
	if (Monster == nullptr)
	{
		return false;
	}

	const float Distance = Monster->GetDistanceToTarget();
	return Distance > 0.0f && Distance <= AttackRange;
}
