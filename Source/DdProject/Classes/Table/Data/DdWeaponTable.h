#pragma once

#include "CoreMinimal.h"
#include "Table/Data/DdJsonTableBase.h"

struct FDdWeaponTableRow
{
	int32 Id = 0;
	FString Type;
	int32 ResourceId = 0;
	FString BoneName;
	FString ComboIds;
	FString Desc;
};

class DDPROJECT_API FDdWeaponTable : public FDdJsonTableBase
{
public:
	FDdWeaponTable();

	const TArray<FDdWeaponTableRow>& GetRows() const { return Rows; }

protected:
	virtual bool DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject) override;
	virtual void ResetRows() override;

private:
	TArray<FDdWeaponTableRow> Rows;
};
