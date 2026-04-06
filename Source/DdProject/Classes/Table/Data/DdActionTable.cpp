#include "Table/Data/DdActionTable.h"

#include "Dom/JsonObject.h"

namespace
{
	const TMap<FString, EDdActionType> ActionTypeMap =
	{
		{ TEXT("NormalAttack"), EDdActionType::NormalAttack },
	};
}

FDdActionTable::FDdActionTable()
	: FDdJsonTableBase(TEXT("ActionTable.json"))
{
}

const FDdActionTableRow* FDdActionTable::FindRowById(int32 Id) const
{
	return Rows.FindByPredicate([Id](const FDdActionTableRow& Row)
	{
		return Row.Id == Id;
	});
}

bool FDdActionTable::DeserializeRow(const TSharedPtr<FJsonObject>& JsonObject)
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

	FDdActionTableRow& Row = Rows.AddDefaulted_GetRef();
	Row.Id = static_cast<int32>(IdValue);
	Row.Type = ParseEnum(GetOptionalStringField(JsonObject, TEXT("eType")), ActionTypeMap, EDdActionType::None);
	Row.ComboResourceId1 = GetOptionalIntField(JsonObject, TEXT("Combo_ResourceID_1"));
	Row.bComboBlend1 = GetOptionalBoolField(JsonObject, TEXT("bCombo_Blend_1"));
	Row.ComboResourceId2 = GetOptionalIntField(JsonObject, TEXT("Combo_ResourceID_2"));
	Row.bComboBlend2 = GetOptionalBoolField(JsonObject, TEXT("bCombo_Blend_2"));
	Row.ComboResourceId3 = GetOptionalIntField(JsonObject, TEXT("Combo_ResourceID_3"));
	Row.bComboBlend3 = GetOptionalBoolField(JsonObject, TEXT("bCombo_Blend_3"));
	Row.ComboResourceId4 = GetOptionalIntField(JsonObject, TEXT("Combo_ResourceID_4"));
	Row.bComboBlend4 = GetOptionalBoolField(JsonObject, TEXT("bCombo_Blend_4"));
	Row.Desc = GetOptionalStringField(JsonObject, TEXT("Desc"));
	return true;
}

void FDdActionTable::ResetRows()
{
	Rows.Reset();
}
