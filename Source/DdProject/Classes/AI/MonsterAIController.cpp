#include "MonsterAIController.h"

#include "BehaviorTree/BehaviorTree.h"
#include "Character/DdMonsterCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Character.h"

AMonsterAIController::AMonsterAIController()
{
	// AIPerception 컴포넌트 생성
	AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));
	SetPerceptionComponent(*AIPerceptionComp);

	// 시야 설정
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngle;
	SightConfig->SetMaxAge(10.0f);
	SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerceptionComp->ConfigureSense(*SightConfig);
	AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// 개별 액터 감지/비감지 콜백 바인딩
	if (AIPerceptionComp != nullptr)
	{
		AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AMonsterAIController::OnTargetPerceptionUpdated);
	}

	// 비헤이비어 트리 실행
	if (BehaviorTreeAsset != nullptr)
	{
		RunBehaviorTree(BehaviorTreeAsset);
	}
}

void AMonsterAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (Actor == nullptr)
	{
		return;
	}

	ADdMonsterCharacter* Monster = Cast<ADdMonsterCharacter>(GetPawn());
	if (Monster == nullptr)
	{
		return;
	}

	// 플레이어 캐릭터인지 확인
	ACharacter* PerceivedCharacter = Cast<ACharacter>(Actor);
	if (PerceivedCharacter == nullptr)
	{
		return;
	}

	AController* ActorController = PerceivedCharacter->GetController();
	if (ActorController == nullptr || !ActorController->IsPlayerController())
	{
		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		// 적 감지 — 타겟 설정
		Monster->SetTargetActor(Actor);
	}
	else
	{
		// 적이 시야에서 벗어남 — 현재 타겟과 같은 액터면 리셋
		if (Monster->GetTargetActor() == Actor)
		{
			Monster->SetTargetActor(nullptr);
		}
	}
}
