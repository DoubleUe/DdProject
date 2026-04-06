#pragma once

#include "CoreMinimal.h"
#include "Table/Data/DdJsonTableBase.h"

struct FDdResourceTableRow
{
	int32 Id = 0;
	FString Path;
	FString Desc;
};

class DDPROJECT_API FDdResourceTable : public FDdJsonTableBase
{
public:
	FDdResourceTable();

	const TArray<FDdResourceTableRow>& GetRows() const { return Rows; }

protected:
	virtual bool DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject) override;
	virtual void ResetRows() override;

private:
	TArray<FDdResourceTableRow> Rows;
};
