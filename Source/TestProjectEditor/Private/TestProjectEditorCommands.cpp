#include "TestProjectEditorCommands.h"

#include "Framework/Commands/InputChord.h"
#include "InputCoreTypes.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "FTestProjectEditorCommands"

FTestProjectEditorCommands::FTestProjectEditorCommands()
	: TCommands<FTestProjectEditorCommands>(
		TEXT("TestProjectEditor"),
		LOCTEXT("ContextDescription", "Test Project Editor"),
		NAME_None,
		FAppStyle::GetAppStyleSetName())
{
}

void FTestProjectEditorCommands::RegisterCommands()
{
	UI_COMMAND(
		OpenTestToolsWindow,
		"Open Test Tools",
		"Open the Test Tools window.",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::RightBracket));
}

#undef LOCTEXT_NAMESPACE
