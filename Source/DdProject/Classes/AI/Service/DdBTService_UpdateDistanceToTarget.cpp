#include "DdBTService_UpdateDistanceToTarget.h"

#include "AIController.h"
#include "Character/DdMonsterCharacter.h"

UDdBTService_UpdateDistanceToTarget::UDdBTService_UpdateDistanceToTarget()
{
	NodeName = TEXT("타겟 거리 갱신");
	Interval = 0.2f;
	RandomDeviation = 0.05f;
}

void UDdBTService_UpdateDistanceToTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController == nullptr)
	{
		return;
	}

	ADdMonsterCharacter* Monster = Cast<ADdMonsterCharacter>(AIController->GetPawn());
	if (Monster == nullptr)
	{
		return;
	}

	AActor* TargetActor = Monster->GetTargetActor();
	if (TargetActor == nullptr)
	{
		Monster->SetDistanceToTarget(0.0f);
		return;
	}

	const float Distance = FVector::Dist(Monster->GetActorLocation(), TargetActor->GetActorLocation());
	Monster->SetDistanceToTarget(Distance);
}
