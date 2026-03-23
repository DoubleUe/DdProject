#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZombieChaseAIController.generated.h"

class ACharacter;

UCLASS()
class DDPROJECT_API AZombieChaseAIController : public AAIController
{
	GENERATED_BODY()

public:
	AZombieChaseAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void RefreshChaseTarget();
	void RequestChase();
	bool ShouldUsePathfinding() const;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float ChaseAcceptanceRadius = 125.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	bool bPreferNavigationPathfinding = true;

	TWeakObjectPtr<ACharacter> TargetCharacter;
};
