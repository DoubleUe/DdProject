#pragma once

#include "CoreMinimal.h"
#include "Table/Data/DdJsonTableBase.h"

struct FDdGameDataTableRow
{
	int32 Id = 0;
	FString Type;
	FString Desc;
};

class DDPROJECT_API FDdGameDataTable : public FDdJsonTableBase
{
public:
	FDdGameDataTable();

	const TArray<FDdGameDataTableRow>& GetRows() const { return Rows; }

protected:
	virtual bool DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject) override;
	virtual void ResetRows() override;

private:
	TArray<FDdGameDataTableRow> Rows;
};
