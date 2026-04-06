#include "Table/Data/DdResourceTable.h"

#include "Dom/JsonObject.h"

FDdResourceTable::FDdResourceTable()
	: FDdJsonTableBase(TEXT("Resource.json"))
{
}

bool FDdResourceTable::DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject)
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

	FDdResourceTableRow& Row = Rows.AddDefaulted_GetRef();
	Row.Id = static_cast<int32>(IdValue);
	Row.Path = GetOptionalStringField(JsonObject, TEXT("Path"));
	Row.Desc = GetOptionalStringField(JsonObject, TEXT("Desc"));
	return true;
}

void FDdResourceTable::ResetRows()
{
	Rows.Reset();
}
