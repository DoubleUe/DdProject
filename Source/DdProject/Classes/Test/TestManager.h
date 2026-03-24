#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestManager.generated.h"

class ACharacter;
class AMonsterCharacter;
class UWorld;

UCLASS()
class DDPROJECT_API ATestManager : public AActor
{
	GENERATED_BODY()

public:
	ATestManager();

protected:
	virtual void BeginPlay() override;

public:
	static ATestManager* FindOrCreate(UWorld* World);

	UPROPERTY(EditAnywhere, Category = "Test")
	TSubclassOf<AMonsterCharacter> ZombieClass;

	UPROPERTY(EditAnywhere, Category = "Test")
	float SpawnRadius = 1000.0f;

	UPROPERTY(EditAnywhere, Category = "Test")
	bool bSpawnZombieOnBeginPlay = false;

	UFUNCTION(BlueprintCallable, Category = "Test")
	AMonsterCharacter* SpawnZombie();

private:
	ACharacter* FindTargetCharacter() const;
	bool TryResolveZombieSpawn(FVector& OutSpawnLocation, FRotator& OutSpawnRotation) const;
};
