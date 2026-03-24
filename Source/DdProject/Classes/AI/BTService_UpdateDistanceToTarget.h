#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateDistanceToTarget.generated.h"

// 타겟까지의 거리를 블랙보드에 갱신하는 서비스
UCLASS(Blueprintable)
class DDPROJECT_API UBTService_UpdateDistanceToTarget : public UBTService
{
	GENERATED_BODY()

public:
	UBTService_UpdateDistanceToTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
