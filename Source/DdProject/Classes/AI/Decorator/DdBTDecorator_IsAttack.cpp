#include "DdBTDecorator_IsAttack.h"

#include "AIController.h"
#include "Character/DdMonsterCharacter.h"

UDdBTDecorator_IsAttack::UDdBTDecorator_IsAttack()
{
	NodeName = TEXT("공격 가능 확인");
}

bool UDdBTDecorator_IsAttack::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
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

	// 공격 범위 내에 있고 공격 가능한 상태인지 확인
	const float Distance = Monster->GetDistanceToTarget();
	const bool bInRange = Distance > 0.0f && Distance <= AttackRange;

	return bInRange && Monster->CanAttack();
}
