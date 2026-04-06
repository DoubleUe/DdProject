#include "DdTestManager.h"

#include "Character/DdMonsterCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "UObject/ConstructorHelpers.h"

ADdTestManager::ADdTestManager()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FClassFinder<ADdMonsterCharacter> ZombieBlueprintClass(TEXT("/Game/Characters/Whisper/CBP_Whisper"));
	if (ZombieBlueprintClass.Succeeded())
	{
		ZombieClass = ZombieBlueprintClass.Class;
	}
	else
	{
		ZombieClass = ADdMonsterCharacter::StaticClass();
	}
}

ADdTestManager* ADdTestManager::FindOrCreate(UWorld* World)
{
	if (World == nullptr)
	{
		return nullptr;
	}

	for (TActorIterator<ADdTestManager> It(World); It; ++It)
	{
		return *It;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = TEXT("DdTestManager");
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (World->WorldType == EWorldType::Editor)
	{
		SpawnParameters.ObjectFlags |= RF_Transient;
	}

	ADdTestManager* SpawnedManager = World->SpawnActor<ADdTestManager>(
		StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);

#if WITH_EDITOR
	if (SpawnedManager != nullptr)
	{
		SpawnedManager->SetActorLabel(TEXT("DdTestManager"));
	}
#endif

	return SpawnedManager;
}

void ADdTestManager::BeginPlay()
{
	Super::BeginPlay();

	if (bSpawnZombieOnBeginPlay)
	{
		SpawnZombie();
	}
}

ADdMonsterCharacter* ADdTestManager::SpawnZombie()
{
	UWorld* World = GetWorld();
	if (World == nullptr || ZombieClass == nullptr)
	{
		return nullptr;
	}

	FVector SpawnLocation = FVector::ZeroVector;
	FRotator SpawnRotation = FRotator::ZeroRotator;
	if (!TryResolveZombieSpawn(SpawnLocation, SpawnRotation))
	{
		return nullptr;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (World->WorldType == EWorldType::Editor)
	{
		SpawnParameters.ObjectFlags |= RF_Transient;
	}

	ADdMonsterCharacter* SpawnedZombie = World->SpawnActor<ADdMonsterCharacter>(
		ZombieClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParameters);

	if (IsValid(SpawnedZombie) && World->IsGameWorld() && SpawnedZombie->GetController() == nullptr)
	{
		SpawnedZombie->SpawnDefaultController();
	}

	return SpawnedZombie;
}

ACharacter* ADdTestManager::FindTargetCharacter() const
{
	UWorld* World = GetWorld();
	return World != nullptr ? UGameplayStatics::GetPlayerCharacter(World, 0) : nullptr;
}

bool ADdTestManager::TryResolveZombieSpawn(FVector& OutSpawnLocation, FRotator& OutSpawnRotation) const
{
	ACharacter* TargetCharacter = FindTargetCharacter();
	if (!IsValid(TargetCharacter))
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	const FVector TargetLocation = TargetCharacter->GetActorLocation();
	const float MinimumSpawnDistance = FMath::Min(SpawnRadius, 200.0f);
	bool bFoundSpawn = false;

	if (UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
	{
		if (NavigationSystem->GetDefaultNavDataInstance(FNavigationSystem::DontCreate) != nullptr)
		{
			for (int32 AttemptIndex = 0; AttemptIndex < 8; ++AttemptIndex)
			{
				FNavLocation CandidateLocation;
				if (NavigationSystem->GetRandomReachablePointInRadius(TargetLocation, SpawnRadius, CandidateLocation)
					&& FVector::DistSquared2D(CandidateLocation.Location, TargetLocation) >= FMath::Square(MinimumSpawnDistance))
				{
					OutSpawnLocation = CandidateLocation.Location;
					bFoundSpawn = true;
					break;
				}
			}
		}
	}

	if (!bFoundSpawn)
	{
		FVector2D RandomOffset = FMath::RandPointInCircle(SpawnRadius);
		if (RandomOffset.IsNearlyZero())
		{
			RandomOffset = FVector2D(MinimumSpawnDistance, 0.0f);
		}
		else if (RandomOffset.SizeSquared() < FMath::Square(MinimumSpawnDistance))
		{
			RandomOffset = RandomOffset.GetSafeNormal() * MinimumSpawnDistance;
		}

		OutSpawnLocation = TargetLocation + FVector(RandomOffset.X, RandomOffset.Y, 0.0f);
		OutSpawnLocation.Z = TargetLocation.Z;
	}

	FVector ToTarget = TargetLocation - OutSpawnLocation;
	ToTarget.Z = 0.0f;
	OutSpawnRotation = ToTarget.IsNearlyZero()
		? TargetCharacter->GetActorRotation()
		: ToTarget.Rotation();

	return true;
}
