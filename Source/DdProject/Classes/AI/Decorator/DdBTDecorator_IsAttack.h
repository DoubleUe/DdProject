#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "DdBTDecorator_IsAttack.generated.h"

// 타겟이 공격 범위 내에 있고 공격 가능한 상태인지 확인하는 데코레이터
UCLASS(Blueprintable)
class DDPROJECT_API UDdBTDecorator_IsAttack : public UBTDecorator
{
	GENERATED_BODY()

public:
	UDdBTDecorator_IsAttack();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	// 공격 범위 (cm)
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 150.0f;
};
