#pragma once

#include "CoreMinimal.h"

class FJsonObject;

class DDPROJECT_API FDdJsonTableBase
{
public:
	explicit FDdJsonTableBase(FString InTableFileName);
	virtual ~FDdJsonTableBase() = default;

	bool Load();
	FString GetTableFilePath() const;

protected:
	static FString GetOptionalStringField(const TSharedPtr<FJsonObject>& JsonObject, const TCHAR* FieldName);
	static int32 GetOptionalIntField(const TSharedPtr<FJsonObject>& JsonObject, const TCHAR* FieldName);

	virtual bool DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject) = 0;
	virtual void ResetRows() = 0;

private:
	FString TableFileName;
};
