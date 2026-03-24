#include "BTTask_ChaseTarget.h"

#include "AIController.h"
#include "Character/MonsterCharacter.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_ChaseTarget::UBTTask_ChaseTarget()
{
	NodeName = TEXT("타겟 추적");
	bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_ChaseTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AMonsterCharacter* Monster = Cast<AMonsterCharacter>(AIController->GetPawn());
	if (Monster == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Monster->GetTargetActor();
	if (TargetActor == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// 비동기 이동 요청
	FAIMoveRequest MoveRequest(TargetActor);
	MoveRequest.SetAcceptanceRadius(AcceptableRadius);
	MoveRequest.SetUsePathfinding(true);

	const FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest);

	if (Result.Code == EPathFollowingRequestResult::RequestSuccessful)
	{
		// 이동 완료 콜백 등록
		CachedOwnerComp = &OwnerComp;
		MoveCompletedHandle = AIController->GetPathFollowingComponent()->OnRequestFinished.AddUObject(
			this, &UBTTask_ChaseTarget::OnMoveCompleted);
		return EBTNodeResult::InProgress;
	}

	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}

EBTNodeResult::Type UBTTask_ChaseTarget::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController != nullptr)
	{
		AIController->StopMovement();

		if (UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent())
		{
			PathComp->OnRequestFinished.Remove(MoveCompletedHandle);
		}
	}

	return EBTNodeResult::Aborted;
}

void UBTTask_ChaseTarget::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController != nullptr)
	{
		if (UPathFollowingComponent* PathComp = AIController->GetPathFollowingComponent())
		{
			PathComp->OnRequestFinished.Remove(MoveCompletedHandle);
		}
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_ChaseTarget::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (CachedOwnerComp.IsValid())
	{
		const bool bSuccess = Result.IsSuccess();
		FinishLatentTask(*CachedOwnerComp, bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed);
	}
}
