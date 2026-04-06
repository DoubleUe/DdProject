#include "Panels/TableEditor/SDdTableEditorPanel.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "HAL/FileManager.h"
#include "Input/Reply.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "SDdTableEditorPanel"

namespace
{
	const FName RowNumberColumnId(TEXT("__RowNumber"));
	const FName MessageColumnId(TEXT("Message"));

	class SDdTableEditorListRow final : public SMultiColumnTableRow<TSharedPtr<FDdTableEditorRowItem>>
	{
	public:
		SLATE_BEGIN_ARGS(SDdTableEditorListRow)
		{
		}
			SLATE_ARGUMENT(TSharedPtr<FDdTableEditorRowItem>, Item)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTableView)
		{
			Item = InArgs._Item;
			SMultiColumnTableRow<TSharedPtr<FDdTableEditorRowItem>>::Construct(
				FSuperRowType::FArguments(),
				OwnerTableView);
		}

		virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
		{
			const FString* CellText = Item.IsValid() ? Item->CellValues.Find(ColumnName) : nullptr;
			return SNew(STextBlock)
				.Text(FText::FromString(CellText != nullptr ? *CellText : FString()));
		}

	private:
		TSharedPtr<FDdTableEditorRowItem> Item;
	};
}

void SDdTableEditorPanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TableEditorLabel", "Table Editor"))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 6.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock)
			.AutoWrapText(true)
			.Text(LOCTEXT("TableEditorDescription", "Select a JSON table under Content/Json/Table and inspect its rows below."))
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 12.0f, 0.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SAssignNew(TableComboBox, SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&TableOptions)
				.InitiallySelectedItem(SelectedTableOption)
				.OnGenerateWidget(this, &SDdTableEditorPanel::GenerateTableOptionWidget)
				.OnSelectionChanged(this, &SDdTableEditorPanel::HandleTableSelectionChanged)
				[
					SNew(STextBlock)
					.Text(this, &SDdTableEditorPanel::GetSelectedTableText)
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshTablesButton", "Refresh"))
				.OnClicked(this, &SDdTableEditorPanel::HandleRefreshTablesClicked)
			]
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 12.0f, 0.0f, 6.0f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TableRowsLabel", "Rows"))
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(0.0f, 4.0f, 0.0f, 0.0f)
		[
			SNew(SBox)
			.MinDesiredHeight(320.0f)
			[
				SAssignNew(TableViewContainer, SBox)
			]
		]
	];

	ReloadTableOptions();
}

void SDdTableEditorPanel::ReloadTableOptions()
{
	TableOptions.Reset();

	TArray<FString> TableFileNames;
	IFileManager::Get().FindFiles(TableFileNames, *GetTableSearchPattern(), true, false);
	TableFileNames.Sort();

	for (const FString& TableFileName : TableFileNames)
	{
		TableOptions.Add(MakeShared<FString>(TableFileName));
	}

	if (TableOptions.Num() > 0)
	{
		if (!SelectedTableOption.IsValid() || !TableOptions.ContainsByPredicate([this](const TSharedPtr<FString>& Option)
		{
			return Option.IsValid() && SelectedTableOption.IsValid() && *Option == *SelectedTableOption;
		}))
		{
			SelectedTableOption = TableOptions[0];
		}
	}
	else
	{
		SelectedTableOption.Reset();
	}

	ReloadSelectedTableRows();

	if (TableComboBox.IsValid())
	{
		TableComboBox->RefreshOptions();
		if (SelectedTableOption.IsValid())
		{
			TableComboBox->SetSelectedItem(SelectedTableOption);
		}
		else
		{
			TableComboBox->ClearSelection();
		}
	}
}

void SDdTableEditorPanel::ReloadSelectedTableRows()
{
	TableRowItems.Reset();
	TableColumnIds.Reset();

	const FString SelectedTablePath = GetSelectedTablePath();
	if (SelectedTablePath.IsEmpty())
	{
		if (TableOptions.Num() == 0)
		{
			SetMessageRow(FString::Printf(TEXT("No JSON tables found in %s"), *GetTableDirectoryPath()));
		}
		else
		{
			SetMessageRow(TEXT("No table selected."));
		}
	}
	else
	{
		FString JsonText;
		if (!FFileHelper::LoadFileToString(JsonText, *SelectedTablePath))
		{
			SetMessageRow(FString::Printf(TEXT("Failed to load table: %s"), *SelectedTablePath));
		}
		else
		{
			TArray<TSharedPtr<FJsonValue>> JsonValues;
			const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonText);

			// 배열 파싱 시도, 실패하면 단일 객체로 시도
			if (!FJsonSerializer::Deserialize(JsonReader, JsonValues))
			{
				TSharedPtr<FJsonObject> SingleObject;
				const TSharedRef<TJsonReader<>> RetryReader = TJsonReaderFactory<>::Create(JsonText);
				if (FJsonSerializer::Deserialize(RetryReader, SingleObject) && SingleObject.IsValid())
				{
					JsonValues.Add(MakeShared<FJsonValueObject>(SingleObject));
				}
			}

			if (JsonValues.Num() == 0)
			{
				SetMessageRow(TEXT("The selected table has no rows."));
			}
			else
			{
				// JSON 키 순서를 유지하면서 컬럼 수집
				TSet<FName> ColumnIdSet;
				TArray<FName> OrderedColumnIds;
				for (int32 Index = 0; Index < JsonValues.Num(); ++Index)
				{
					const TSharedPtr<FJsonObject> JsonObject = JsonValues[Index].IsValid() ? JsonValues[Index]->AsObject() : nullptr;
					if (!JsonObject.IsValid())
					{
						continue;
					}

					for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : JsonObject->Values)
					{
						const FName ColumnId(*Pair.Key);
						if (!ColumnIdSet.Contains(ColumnId))
						{
							ColumnIdSet.Add(ColumnId);
							OrderedColumnIds.Add(ColumnId);
						}
					}
				}

				TableColumnIds.Add(RowNumberColumnId);
				TableColumnIds.Append(OrderedColumnIds);

				for (int32 Index = 0; Index < JsonValues.Num(); ++Index)
				{
					const TSharedPtr<FJsonObject> JsonObject = JsonValues[Index].IsValid() ? JsonValues[Index]->AsObject() : nullptr;
					if (!JsonObject.IsValid())
					{
						continue;
					}

					TSharedPtr<FDdTableEditorRowItem> RowItem = MakeShared<FDdTableEditorRowItem>();
					RowItem->CellValues.Add(RowNumberColumnId, FString::FromInt(Index + 1));

					for (const FName& ColumnId : TableColumnIds)
					{
						if (ColumnId == RowNumberColumnId)
						{
							continue;
						}

						const TSharedPtr<FJsonValue>* JsonValue = JsonObject->Values.Find(ColumnId.ToString());
						RowItem->CellValues.Add(ColumnId, JsonValue != nullptr ? FormatJsonValue(*JsonValue) : FString());
					}

					TableRowItems.Add(RowItem);
				}

				if (TableRowItems.Num() == 0)
				{
					SetMessageRow(TEXT("No valid rows were found in the selected table."));
				}
			}
		}
	}

	RebuildTableView();

	if (TableRowListView.IsValid())
	{
		TableRowListView->RequestListRefresh();
	}
}

void SDdTableEditorPanel::RebuildTableView()
{
	if (!TableViewContainer.IsValid())
	{
		return;
	}

	TSharedRef<SHeaderRow> HeaderRowWidget = SNew(SHeaderRow);
	for (const FName& ColumnId : TableColumnIds)
	{
		SHeaderRow::FColumn::FArguments ColumnArguments = SHeaderRow::Column(ColumnId)
			.DefaultLabel(GetColumnDisplayText(ColumnId));

		if (ColumnId == RowNumberColumnId)
		{
			ColumnArguments.FixedWidth(56.0f);
		}
		else
		{
			ColumnArguments.FillWidth(1.0f);
		}

		HeaderRowWidget->AddColumn(ColumnArguments);
	}

	TSharedRef<SListView<TSharedPtr<FDdTableEditorRowItem>>> ListViewWidget =
		SNew(SListView<TSharedPtr<FDdTableEditorRowItem>>)
		.ListItemsSource(&TableRowItems)
		.OnGenerateRow(this, &SDdTableEditorPanel::GenerateTableRowWidget)
		.SelectionMode(ESelectionMode::None)
		.HeaderRow(HeaderRowWidget);

	TableRowListView = ListViewWidget;
	TableViewContainer->SetContent(ListViewWidget);
}

void SDdTableEditorPanel::SetMessageRow(const FString& Message)
{
	TableColumnIds = { MessageColumnId };

	TSharedPtr<FDdTableEditorRowItem> RowItem = MakeShared<FDdTableEditorRowItem>();
	RowItem->CellValues.Add(MessageColumnId, Message);
	TableRowItems.Add(RowItem);
}

FReply SDdTableEditorPanel::HandleRefreshTablesClicked()
{
	ReloadTableOptions();
	return FReply::Handled();
}

void SDdTableEditorPanel::HandleTableSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
	SelectedTableOption = NewSelection;
	ReloadSelectedTableRows();
}

TSharedRef<SWidget> SDdTableEditorPanel::GenerateTableOptionWidget(TSharedPtr<FString> InItem) const
{
	return SNew(STextBlock)
		.Text(FText::FromString(InItem.IsValid() ? *InItem : FString()));
}

TSharedRef<ITableRow> SDdTableEditorPanel::GenerateTableRowWidget(TSharedPtr<FDdTableEditorRowItem> InItem, const TSharedRef<STableViewBase>& OwnerTable) const
{
	return SNew(SDdTableEditorListRow, OwnerTable)
		.Item(InItem);
}

FText SDdTableEditorPanel::GetSelectedTableText() const
{
	if (!SelectedTableOption.IsValid())
	{
		return TableOptions.Num() == 0
			? LOCTEXT("NoTablesFound", "No tables found")
			: LOCTEXT("NoTableSelected", "Select a table");
	}

	return FText::FromString(*SelectedTableOption);
}

FString SDdTableEditorPanel::GetTableDirectoryPath() const
{
	return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Json"), TEXT("Table"));
}

FString SDdTableEditorPanel::GetTableSearchPattern() const
{
	return FPaths::Combine(GetTableDirectoryPath(), TEXT("*.json"));
}

FString SDdTableEditorPanel::GetSelectedTablePath() const
{
	if (!SelectedTableOption.IsValid())
	{
		return FString();
	}

	return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Json"), TEXT("Table"), *SelectedTableOption);
}

FString SDdTableEditorPanel::FormatJsonValue(const TSharedPtr<FJsonValue>& JsonValue) const
{
	if (!JsonValue.IsValid())
	{
		return TEXT("null");
	}

	switch (JsonValue->Type)
	{
	case EJson::None:
	case EJson::Null:
		return TEXT("null");
	case EJson::String:
		return JsonValue->AsString();
	case EJson::Number:
		return JsonValue->AsString();
	case EJson::Boolean:
		return JsonValue->AsBool() ? TEXT("true") : TEXT("false");
	case EJson::Array:
	{
		TArray<FString> ValueStrings;
		for (const TSharedPtr<FJsonValue>& Value : JsonValue->AsArray())
		{
			ValueStrings.Add(FormatJsonValue(Value));
		}
		return FString::Printf(TEXT("[%s]"), *FString::Join(ValueStrings, TEXT(", ")));
	}
	case EJson::Object:
	{
		TArray<FString> ObjectFields;
		const TSharedPtr<FJsonObject> JsonObject = JsonValue->AsObject();
		if (!JsonObject.IsValid())
		{
			return TEXT("{}");
		}

		TArray<FString> Keys;
		JsonObject->Values.GetKeys(Keys);
		Keys.Sort();

		for (const FString& Key : Keys)
		{
			const TSharedPtr<FJsonValue>* Value = JsonObject->Values.Find(Key);
			ObjectFields.Add(FString::Printf(TEXT("%s: %s"), *Key, Value != nullptr ? *FormatJsonValue(*Value) : TEXT("null")));
		}

		return FString::Printf(TEXT("{%s}"), *FString::Join(ObjectFields, TEXT(", ")));
	}
	default:
		return TEXT("<unsupported>");
	}
}

FText SDdTableEditorPanel::GetColumnDisplayText(const FName& ColumnId) const
{
	if (ColumnId == RowNumberColumnId)
	{
		return LOCTEXT("RowNumberColumn", "Row");
	}

	FString ColumnName = ColumnId.ToString();

	// e 접두사(enum), b 접두사(boolean) 제거하여 표시
	if (ColumnName.Len() > 1 && FChar::IsUpper(ColumnName[1]))
	{
		if (ColumnName[0] == TEXT('e'))
		{
			ColumnName = ColumnName.Mid(1) + TEXT(" (enum)");
		}
		else if (ColumnName[0] == TEXT('b'))
		{
			ColumnName = ColumnName.Mid(1) + TEXT(" (bool)");
		}
	}

	return FText::FromString(ColumnName);
}

#undef LOCTEXT_NAMESPACE
