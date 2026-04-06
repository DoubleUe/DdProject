#include "Table/Data/DdWeaponTable.h"

#include "Dom/JsonObject.h"

FDdWeaponTable::FDdWeaponTable()
	: FDdJsonTableBase(TEXT("Weapon.json"))
{
}

const FDdWeaponTableRow* FDdWeaponTable::FindRowById(int32 Id) const
{
	return Rows.FindByPredicate([Id](const FDdWeaponTableRow& Row)
	{
		return Row.Id == Id;
	});
}

bool FDdWeaponTable::DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject)
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

	FDdWeaponTableRow& Row = Rows.AddDefaulted_GetRef();
	Row.Id = static_cast<int32>(IdValue);
	Row.Type = GetOptionalStringField(JsonObject, TEXT("Type"));
	Row.ResourceId = GetOptionalIntField(JsonObject, TEXT("ResourceID"));
	Row.WeaponBoneName = GetOptionalStringField(JsonObject, TEXT("WeaponBoneName"));
	Row.CharacterBoneName = GetOptionalStringField(JsonObject, TEXT("CharacterBoneName"));
	Row.ComboIds = GetOptionalStringField(JsonObject, TEXT("ComboIDs"));
	Row.Desc = GetOptionalStringField(JsonObject, TEXT("Desc"));
	return true;
}

void FDdWeaponTable::ResetRows()
{
	Rows.Reset();
}
