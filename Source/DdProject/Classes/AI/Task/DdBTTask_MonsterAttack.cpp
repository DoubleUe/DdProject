#include "DdBTTask_MonsterAttack.h"

#include "AIController.h"
#include "Character/DdMonsterCharacter.h"

UDdBTTask_MonsterAttack::UDdBTTask_MonsterAttack()
{
	NodeName = TEXT("몬스터 공격");
}

EBTNodeResult::Type UDdBTTask_MonsterAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	ADdMonsterCharacter* Monster = Cast<ADdMonsterCharacter>(AIController->GetPawn());
	if (Monster == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// 공격 애니메이션 재생 성공하면 즉시 태스크 완료
	if (!Monster->PlayAttackAnimation())
	{
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::Succeeded;
}
