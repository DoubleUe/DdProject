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
	static bool GetOptionalBoolField(const TSharedPtr<FJsonObject>& JsonObject, const TCHAR* FieldName);

	// enum 파싱 헬퍼 — 자식 클래스에서 ValueMap을 정의하여 사용
	template<typename TEnum>
	static TEnum ParseEnum(const FString& Value, const TMap<FString, TEnum>& ValueMap, TEnum DefaultValue)
	{
		const TEnum* Found = ValueMap.Find(Value);
		return Found != nullptr ? *Found : DefaultValue;
	}

	virtual bool DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject) = 0;
	virtual void ResetRows() = 0;

private:
	FString TableFileName;
};
