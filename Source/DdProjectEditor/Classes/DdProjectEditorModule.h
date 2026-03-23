#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "Modules/ModuleInterface.h"

class FSpawnTabArgs;
class IInputProcessor;
class FUICommandList;
class SDockTab;
class UWorld;

class FDdProjectEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void OpenTestToolsWindow();
	void RunZombiePatrolTest();
	TSharedRef<SDockTab> SpawnTestToolsTab(const FSpawnTabArgs& SpawnTabArgs);
	UWorld* GetTargetWorld() const;
	void ShowNotification(const FText& Message) const;

	static const FName TestToolsTabName;

	TSharedPtr<FUICommandList> EditorCommands;
	TSharedPtr<IInputProcessor> ShortcutInputProcessor;
};
