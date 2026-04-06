#include "DdCombatComponent.h"

#include "Animation/AnimMontage.h"
#include "Table/Data/DdActionTable.h"
#include "Table/Data/DdResourceTable.h"
#include "Table/DdTableSubsystem.h"

UDdCombatComponent::UDdCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDdCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UDdCombatComponent::LoadComboData(const TArray<int32>& ComboIds, UDdTableSubsystem* TableSubsystem)
{
	ComboSets.Reset();

	if (TableSubsystem == nullptr || !TableSubsystem->IsInitialized())
	{
		UE_LOG(LogTemp, Warning, TEXT("CombatComponent: TableSubsystem is not ready."));
		return;
	}

	const FDdActionTable& ActionTable = TableSubsystem->GetActionTable();

	for (const int32 ComboId : ComboIds)
	{
		const FDdActionTableRow* ActionRow = ActionTable.FindRowById(ComboId);
		if (ActionRow == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("CombatComponent: ActionTable row not found for ComboId %d."), ComboId);
			continue;
		}

		FDdComboSet ComboSet;
		ComboSet.ActionId = ComboId;

		if (LoadComboStepsFromActionRow(*ActionRow, TableSubsystem, ComboSet))
		{
			ComboSets.Add(MoveTemp(ComboSet));
		}
	}

	UE_LOG(LogTemp, Log, TEXT("CombatComponent: Loaded %d combo set(s)."), ComboSets.Num());
}

bool UDdCombatComponent::LoadComboStepsFromActionRow(const FDdActionTableRow& ActionRow, UDdTableSubsystem* TableSubsystem, FDdComboSet& OutComboSet)
{
	const FDdResourceTable& ResourceTable = TableSubsystem->GetResourceTable();

	// 콤보 리소스ID와 블렌드 정보 배열화
	struct FComboEntry
	{
		int32 ResourceId;
		bool bUseBlend;
	};

	const FComboEntry Entries[] =
	{
		{ ActionRow.ComboResourceId1, ActionRow.bComboBlend1 },
		{ ActionRow.ComboResourceId2, ActionRow.bComboBlend2 },
		{ ActionRow.ComboResourceId3, ActionRow.bComboBlend3 },
		{ ActionRow.ComboResourceId4, ActionRow.bComboBlend4 },
	};

	for (const FComboEntry& Entry : Entries)
	{
		if (Entry.ResourceId <= 0)
		{
			continue;
		}

		const FDdResourceTableRow* ResourceRow = ResourceTable.FindRowById(Entry.ResourceId);
		if (ResourceRow == nullptr || ResourceRow->Path.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("CombatComponent: Resource row not found for ResourceId %d."), Entry.ResourceId);
			continue;
		}

		UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, *ResourceRow->Path);
		if (Montage == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("CombatComponent: Failed to load montage from path: %s"), *ResourceRow->Path);
			continue;
		}

		FDdComboStep Step;
		Step.Montage = Montage;
		Step.bUseBlend = Entry.bUseBlend;
		OutComboSet.Steps.Add(Step);
	}

	return OutComboSet.Steps.Num() > 0;
}
