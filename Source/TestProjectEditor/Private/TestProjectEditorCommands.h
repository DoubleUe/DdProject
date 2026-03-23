#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FTestProjectEditorCommands : public TCommands<FTestProjectEditorCommands>
{
public:
	FTestProjectEditorCommands();

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> OpenTestToolsWindow;
};
