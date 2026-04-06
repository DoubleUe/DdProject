#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FDdProjectEditorCommands : public TCommands<FDdProjectEditorCommands>
{
public:
	FDdProjectEditorCommands();

	virtual void RegisterCommands() override;

	TSharedPtr<FUICommandInfo> OpenZombieSpawnWindow;
	TSharedPtr<FUICommandInfo> OpenTableEditorWindow;
};
