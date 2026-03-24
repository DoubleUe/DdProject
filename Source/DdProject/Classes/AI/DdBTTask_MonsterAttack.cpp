#include "DdBTTask_MonsterAttack.h"

#include "AIController.h"
#include "Character/DdMonsterCharacter.h"

UDdBTTask_MonsterAttack::UDdBTTask_MonsterAttack()
{
	NodeName = TEXT("몬스터 공격");
	bNotifyTick = true;
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

	// 공격 애니메이션 재생 요청
	Monster->PlayAttackAnimation();

	// 공격 애니메이션이 끝날 때까지 대기
	return EBTNodeResult::InProgress;
}

void UDdBTTask_MonsterAttack::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController == nullptr)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ADdMonsterCharacter* Monster = Cast<ADdMonsterCharacter>(AIController->GetPawn());
	if (Monster == nullptr)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 공격 애니메이션이 끝나면 태스크 완료
	if (!Monster->IsAttacking())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
