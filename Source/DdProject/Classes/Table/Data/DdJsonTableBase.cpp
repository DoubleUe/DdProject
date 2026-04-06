#include "Table/Data/DdJsonTableBase.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FDdJsonTableBase::FDdJsonTableBase(FString InTableFileName)
	: TableFileName(MoveTemp(InTableFileName))
{
}

FString FDdJsonTableBase::GetOptionalStringField(const TSharedPtr<FJsonObject>& JsonObject, const TCHAR* FieldName)
{
	FString Value;
	return JsonObject.IsValid() && JsonObject->TryGetStringField(FieldName, Value) ? Value : FString();
}

int32 FDdJsonTableBase::GetOptionalIntField(const TSharedPtr<FJsonObject>& JsonObject, const TCHAR* FieldName)
{
	double NumberValue = 0.0;
	return JsonObject.IsValid() && JsonObject->TryGetNumberField(FieldName, NumberValue) ? static_cast<int32>(NumberValue) : 0;
}

bool FDdJsonTableBase::Load()
{
	ResetRows();

	const FString TableFilePath = GetTableFilePath();
	FString JsonText;
	if (!FFileHelper::LoadFileToString(JsonText, *TableFilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load table json file: %s"), *TableFilePath);
		return false;
	}

	TArray<TSharedPtr<FJsonValue>> JsonValues;
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(JsonReader, JsonValues))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to parse table json file: %s"), *TableFilePath);
		return false;
	}

	for (int32 Index = 0; Index < JsonValues.Num(); ++Index)
	{
		const TSharedPtr<FJsonObject> JsonObject = JsonValues[Index].IsValid() ? JsonValues[Index]->AsObject() : nullptr;
		if (!JsonObject.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid row object found in %s at index %d."), *TableFilePath, Index);
			ResetRows();
			return false;
		}

		if (!DeserializeRow(JsonObject))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to deserialize row in %s at index %d."), *TableFilePath, Index);
			ResetRows();
			return false;
		}
	}

	return true;
}

FString FDdJsonTableBase::GetTableFilePath() const
{
	return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Json"), TEXT("Table"), TableFileName);
}
