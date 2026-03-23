#include "TestManager.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "ZombiePatrolTestCharacter.h"

ATestManager::ATestManager()
{
	PrimaryActorTick.bCanEverTick = false;
	PatrolZombieClass = AZombiePatrolTestCharacter::StaticClass();
}

ATestManager* ATestManager::FindOrCreate(UWorld* World)
{
	if (World == nullptr)
	{
		return nullptr;
	}

	for (TActorIterator<ATestManager> It(World); It; ++It)
	{
		return *It;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = TEXT("TestManager");
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (World->WorldType == EWorldType::Editor)
	{
		SpawnParameters.bTemporaryEditorActor = true;
		SpawnParameters.ObjectFlags |= RF_Transient;
	}

	ATestManager* SpawnedManager = World->SpawnActor<ATestManager>(
		StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);

#if WITH_EDITOR
	if (SpawnedManager != nullptr)
	{
		SpawnedManager->SetActorLabel(TEXT("TestManager"));
	}
#endif

	return SpawnedManager;
}

void ATestManager::BeginPlay()
{
	Super::BeginPlay();

	if (bRunZombiePatrolTestOnBeginPlay)
	{
		RunZombiePatrolTest();
	}
}

void ATestManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopZombiePatrolTest();

	Super::EndPlay(EndPlayReason);
}

AZombiePatrolTestCharacter* ATestManager::RunZombiePatrolTest()
{
	if (IsValid(SpawnedPatrolZombie))
	{
		return SpawnedPatrolZombie;
	}

	UWorld* World = GetWorld();
	if (World == nullptr || PatrolZombieClass == nullptr)
	{
		return nullptr;
	}

	const FVector SpawnLocation = GetActorLocation() + SpawnOffset;
	const FRotator SpawnRotation = GetActorRotation();
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (World->WorldType == EWorldType::Editor)
	{
		SpawnParameters.bTemporaryEditorActor = true;
		SpawnParameters.ObjectFlags |= RF_Transient;
	}

	SpawnedPatrolZombie = World->SpawnActor<AZombiePatrolTestCharacter>(
		PatrolZombieClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);

	return SpawnedPatrolZombie;
}

void ATestManager::StopZombiePatrolTest()
{
	if (!IsValid(SpawnedPatrolZombie))
	{
		return;
	}

	SpawnedPatrolZombie->Destroy();
	SpawnedPatrolZombie = nullptr;
}

bool ATestManager::IsZombiePatrolTestRunning() const
{
	return IsValid(SpawnedPatrolZombie);
}
