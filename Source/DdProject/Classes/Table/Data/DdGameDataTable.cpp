#include "Table/Data/DdGameDataTable.h"

#include "Dom/JsonObject.h"

FDdGameDataTable::FDdGameDataTable()
	: FDdJsonTableBase(TEXT("GameData.json"))
{
}

bool FDdGameDataTable::DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject)
{
	if (!JsonObject.IsValid())
	{
		return false;
	}

	double IdValue = 0.0;
	if (!JsonObject->TryGetNumberField(TEXT("id"), IdValue))
	{
		return false;
	}

	FDdGameDataTableRow& Row = Rows.AddDefaulted_GetRef();
	Row.Id = static_cast<int32>(IdValue);
	Row.Type = GetOptionalStringField(JsonObject, TEXT("Type"));
	Row.Desc = GetOptionalStringField(JsonObject, TEXT("Desc"));
	return true;
}

void FDdGameDataTable::ResetRows()
{
	Rows.Reset();
}
