#include "DdProjectEditorCommands.h"

#include "Framework/Commands/InputChord.h"
#include "InputCoreTypes.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FDdProjectEditorCommands"

FDdProjectEditorCommands::FDdProjectEditorCommands()
	: TCommands<FDdProjectEditorCommands>(
		TEXT("DdProjectEditor"),
		LOCTEXT("ContextDescription", "DdProject Editor Windows"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FDdProjectEditorCommands::RegisterCommands()
{
	UI_COMMAND(
		OpenZombieSpawnWindow,
		"Open Zombie Spawn",
		"Open the zombie spawn window.",
		EUserInterfaceActionType::Button,
		FInputChord());

	UI_COMMAND(
		OpenTableEditorWindow,
		"Open Table Editor",
		"Open the table editor window.",
		EUserInterfaceActionType::Button,
		FInputChord());
}

#undef LOCTEXT_NAMESPACE
