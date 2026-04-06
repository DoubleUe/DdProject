#pragma once

#include "CoreMinimal.h"
#include "Table/Data/DdJsonTableBase.h"

enum class EDdWeaponType : uint8
{
	None,
	Melee,
	Ranged,
};

struct FDdWeaponTableRow
{
	int32 Id = 0;
	EDdWeaponType Type = EDdWeaponType::None;
	int32 ResourceId = 0;
	FString WeaponBoneName;
	FString CharacterBoneName;
	FString ComboIds;
	FString Desc;
};

class DDPROJECT_API FDdWeaponTable : public FDdJsonTableBase
{
public:
	FDdWeaponTable();

	const TArray<FDdWeaponTableRow>& GetRows() const { return Rows; }
	const FDdWeaponTableRow* FindRowById(int32 Id) const;

protected:
	virtual bool DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject) override;
	virtual void ResetRows() override;

private:
	TArray<FDdWeaponTableRow> Rows;
};
