#include "Table/DdTableSubsystem.h"

void UDdTableSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (bIsInitialized)
	{
		return;
	}

	const bool bActionLoaded = InitializeActionTable();
	const bool bGameDataLoaded = InitializeGameDataTable();
	const bool bResourceLoaded = InitializeResourceTable();
	const bool bWeaponLoaded = InitializeWeaponTable();

	bIsInitialized = bActionLoaded && bGameDataLoaded && bResourceLoaded && bWeaponLoaded;

	if (bIsInitialized)
	{
		UE_LOG(LogTemp, Log, TEXT("DdTableSubsystem initialized successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DdTableSubsystem initialization failed."));
	}
}

const FDdGameDataTable& UDdTableSubsystem::GetGameDataTable() const
{
	check(bIsInitialized);
	return GameDataTable;
}

const FDdResourceTable& UDdTableSubsystem::GetResourceTable() const
{
	check(bIsInitialized);
	return ResourceTable;
}

const FDdWeaponTable& UDdTableSubsystem::GetWeaponTable() const
{
	check(bIsInitialized);
	return WeaponTable;
}

const FDdActionTable& UDdTableSubsystem::GetActionTable() const
{
	check(bIsInitialized);
	return ActionTable;
}

bool UDdTableSubsystem::InitializeActionTable()
{
	const bool bLoaded = ActionTable.Load();
	UE_LOG(LogTemp, Log, TEXT("Action table load result: %s"), bLoaded ? TEXT("Success") : TEXT("Failed"));
	return bLoaded;
}

bool UDdTableSubsystem::InitializeGameDataTable()
{
	const bool bLoaded = GameDataTable.Load();
	UE_LOG(LogTemp, Log, TEXT("GameData table load result: %s"), bLoaded ? TEXT("Success") : TEXT("Failed"));
	return bLoaded;
}

bool UDdTableSubsystem::InitializeResourceTable()
{
	const bool bLoaded = ResourceTable.Load();
	UE_LOG(LogTemp, Log, TEXT("Resource table load result: %s"), bLoaded ? TEXT("Success") : TEXT("Failed"));
	return bLoaded;
}

bool UDdTableSubsystem::InitializeWeaponTable()
{
	const bool bLoaded = WeaponTable.Load();
	UE_LOG(LogTemp, Log, TEXT("Weapon table load result: %s"), bLoaded ? TEXT("Success") : TEXT("Failed"));
	return bLoaded;
}
