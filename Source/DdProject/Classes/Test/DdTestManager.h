#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DdTestManager.generated.h"

class ACharacter;
class ADdMonsterCharacter;
class UWorld;

UCLASS()
class DDPROJECT_API ADdTestManager : public AActor
{
	GENERATED_BODY()

public:
	ADdTestManager();

protected:
	virtual void BeginPlay() override;

public:
	static ADdTestManager* FindOrCreate(UWorld* World);

	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<ADdMonsterCharacter> ZombieClass;

	UPROPERTY(EditAnywhere, Category = "Test")
	float SpawnRadius = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Test")
	bool bSpawnZombieOnBeginPlay = false;

	UFUNCTION(BlueprintCallable, Category = "Test")
	ADdMonsterCharacter* SpawnZombie();

private:
	ACharacter* FindTargetCharacter() const;
	bool TryResolveZombieSpawn(FVector& OutSpawnLocation, FRotator& OutSpawnRotation) const;
};
