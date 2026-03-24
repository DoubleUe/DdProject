#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_ChaseTarget.generated.h"

// 타겟을 향해 비동기 이동하는 태스크
UCLASS(Blueprintable)
class DDPROJECT_API UBTTask_ChaseTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_ChaseTarget();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

	// 도착 허용 거리 (cm)
	UPROPERTY(EditAnywhere, Category = "Movement")
	float AcceptableRadius = 100.0f;

private:
	// 이동 완료 콜백
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

	// 현재 이동 중인 BT 컴포넌트 (콜백에서 사용)
	TWeakObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
	FDelegateHandle MoveCompletedHandle;
};
