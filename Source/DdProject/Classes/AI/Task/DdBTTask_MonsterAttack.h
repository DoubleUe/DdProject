#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "DdBTTask_MonsterAttack.generated.h"

// 몬스터 캐릭터에게 공격 애니메이션 재생을 요청하는 태스크
UCLASS(Blueprintable)
class DDPROJECT_API UDdBTTask_MonsterAttack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UDdBTTask_MonsterAttack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
