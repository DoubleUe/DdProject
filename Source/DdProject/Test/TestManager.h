#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestManager.generated.h"

class AZombiePatrolTestCharacter;
class UWorld;

UCLASS()
class DDPROJECT_API ATestManager : public AActor
{
	GENERATED_BODY()

public:
	ATestManager();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	static ATestManager* FindOrCreate(UWorld* World);

	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<AZombiePatrolTestCharacter> PatrolZombieClass;

	UPROPERTY(EditAnywhere, Category = "Test")
	FVector SpawnOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Test")
	bool bRunZombiePatrolTestOnBeginPlay = false;

	UFUNCTION(BlueprintCallable, Category = "Test")
	AZombiePatrolTestCharacter* RunZombiePatrolTest();

	UFUNCTION(BlueprintCallable, Category = "Test")
	void StopZombiePatrolTest();

	UFUNCTION(BlueprintPure, Category = "Test")
	bool IsZombiePatrolTestRunning() const;

private:
	UPROPERTY(Transient)
	TObjectPtr<AZombiePatrolTestCharacter> SpawnedPatrolZombie;
};
