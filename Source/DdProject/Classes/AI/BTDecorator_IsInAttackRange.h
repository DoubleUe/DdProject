#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_IsInAttackRange.generated.h"

// 타겟이 공격 범위 내에 있는지 확인하는 데코레이터
UCLASS(Blueprintable)
class DDPROJECT_API UBTDecorator_IsInAttackRange : public UBTDecorator
{
	GENERATED_BODY()

public:
	UBTDecorator_IsInAttackRange();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	// 공격 범위 (cm)
	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackRange = 150.0f;
};
