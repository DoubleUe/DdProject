#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "Modules/ModuleInterface.h"

class FSpawnTabArgs;
class FUICommandList;
class SDockTab;
class UWorld;

class FDdProjectEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void OpenZombieSpawnWindow();
	void OpenTableEditorWindow();
	void SpawnZombieFromZombieSpawnPanel();
	TSharedRef<SDockTab> SpawnZombieSpawnTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnTableEditorTab(const FSpawnTabArgs& SpawnTabArgs);
	UWorld* GetTargetWorld() const;
	void ShowNotification(const FText& Message) const;

	static const FName ZombieSpawnTabName;
	static const FName TableEditorTabName;

	TSharedPtr<FUICommandList> EditorCommands;
};
