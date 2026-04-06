#pragma once

#include "CoreMinimal.h"
#include "Table/Data/DdJsonTableBase.h"

enum class EDdActionType : uint8
{
	None,
	NormalAttack,
};

struct FDdActionTableRow
{
	int32 Id = 0;
	EDdActionType Type = EDdActionType::None;
	int32 ComboResourceId1 = 0;
	bool bComboBlend1 = false;
	FString Desc;
};

class DDPROJECT_API FDdActionTable : public FDdJsonTableBase
{
public:
	FDdActionTable();

	const TArray<FDdActionTableRow>& GetRows() const { return Rows; }
	const FDdActionTableRow* FindRowById(int32 Id) const;

protected:
	virtual bool DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject) override;
	virtual void ResetRows() override;

private:
	TArray<FDdActionTableRow> Rows;
};
