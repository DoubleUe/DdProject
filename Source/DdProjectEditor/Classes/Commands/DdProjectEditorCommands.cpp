#include "DdProjectEditorCommands.h"

#include "Framework/Commands/InputChord.h"
#include "InputCoreTypes.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FDdProjectEditorCommands"

FDdProjectEditorCommands::FDdProjectEditorCommands()
	: TCommands<FDdProjectEditorCommands>(
		TEXT("DdProjectEditor"),
		LOCTEXT("ContextDescription", "DdProject Zombie Spawn Tools"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FDdProjectEditorCommands::RegisterCommands()
{
	UI_COMMAND(
		OpenTestToolsWindow,
		"Open Zombie Spawn Tools",
		"Open the zombie spawn tools window.",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::RightBracket));
}

#undef LOCTEXT_NAMESPACE
