#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

struct FDdTableEditorRowItem
{
	TMap<FName, FString> CellValues;
};

class ITableRow;
template<typename ItemType> class SListView;
template<typename OptionType> class SComboBox;
class STableViewBase;
class SBox;

class SDdTableEditorPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDdTableEditorPanel)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void ReloadTableOptions();
	void ReloadSelectedTableRows();
	void RebuildTableView();
	void SetMessageRow(const FString& Message);
	FReply HandleRefreshTablesClicked();
	void HandleTableSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
	TSharedRef<SWidget> GenerateTableOptionWidget(TSharedPtr<FString> InItem) const;
	TSharedRef<ITableRow> GenerateTableRowWidget(TSharedPtr<FDdTableEditorRowItem> InItem, const TSharedRef<STableViewBase>& OwnerTable) const;
	FText GetSelectedTableText() const;
	FString GetTableDirectoryPath() const;
	FString GetTableSearchPattern() const;
	FString GetSelectedTablePath() const;
	FString FormatJsonValue(const TSharedPtr<class FJsonValue>& JsonValue) const;
	FText GetColumnDisplayText(const FName& ColumnId) const;

	TArray<TSharedPtr<FString>> TableOptions;
	TArray<TSharedPtr<FDdTableEditorRowItem>> TableRowItems;
	TArray<FName> TableColumnIds;
	TSharedPtr<FString> SelectedTableOption;
	TSharedPtr<SComboBox<TSharedPtr<FString>>> TableComboBox;
	TSharedPtr<SListView<TSharedPtr<FDdTableEditorRowItem>>> TableRowListView;
	TSharedPtr<SBox> TableViewContainer;
};
