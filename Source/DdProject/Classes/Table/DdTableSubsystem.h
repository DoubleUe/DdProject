#pragma once

#include "CoreMinimal.h"
#include "Table/Data/DdGameDataTable.h"
#include "Table/Data/DdResourceTable.h"
#include "Table/Data/DdWeaponTable.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DdTableSubsystem.generated.h"

UCLASS()
class DDPROJECT_API UDdTableSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	bool IsInitialized() const { return bIsInitialized; }

	const FDdGameDataTable& GetGameDataTable() const;
	const FDdResourceTable& GetResourceTable() const;
	const FDdWeaponTable& GetWeaponTable() const;

private:
	bool InitializeGameDataTable();
	bool InitializeResourceTable();
	bool InitializeWeaponTable();

	FDdGameDataTable GameDataTable;
	FDdResourceTable ResourceTable;
	FDdWeaponTable WeaponTable;
	bool bIsInitialized = false;
};
