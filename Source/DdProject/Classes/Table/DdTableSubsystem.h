#pragma once

#include "CoreMinimal.h"
#include "Table/Data/DdActionTable.h"
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

	const FDdActionTable& GetActionTable() const;
	const FDdGameDataTable& GetGameDataTable() const;
	const FDdResourceTable& GetResourceTable() const;
	const FDdWeaponTable& GetWeaponTable() const;

private:
	bool InitializeActionTable();
	bool InitializeGameDataTable();
	bool InitializeResourceTable();
	bool InitializeWeaponTable();

	FDdActionTable ActionTable;
	FDdGameDataTable GameDataTable;
	FDdResourceTable ResourceTable;
	FDdWeaponTable WeaponTable;
	bool bIsInitialized = false;
};
