#include "DdProjectEditorModule.h"

#include "Editor.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "LevelEditor.h"
#include "Commands/DdProjectEditorCommands.h"
#include "Modules/ModuleManager.h"
#include "Panels/TableEditor/SDdTableEditorPanel.h"
#include "Panels/ZombieSpawn/SDdZombieSpawnPanel.h"
#include "Test/DdTestManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FDdProjectEditorModule"

const FName FDdProjectEditorModule::ZombieSpawnTabName(TEXT("DdProjectEditor_ZombieSpawn"));
const FName FDdProjectEditorModule::TableEditorTabName(TEXT("DdProjectEditor_TableEditor"));

IMPLEMENT_MODULE(FDdProjectEditorModule, DdProjectEditor)

void FDdProjectEditorModule::StartupModule()
{
	if (IsRunningCommandlet())
	{
		return;
	}

	FDdProjectEditorCommands::Register();

	EditorCommands = MakeShared<FUICommandList>();
	EditorCommands->MapAction(
		FDdProjectEditorCommands::Get().OpenZombieSpawnWindow,
		FExecuteAction::CreateRaw(this, &FDdProjectEditorModule::OpenZombieSpawnWindow));
	EditorCommands->MapAction(
		FDdProjectEditorCommands::Get().OpenTableEditorWindow,
		FExecuteAction::CreateRaw(this, &FDdProjectEditorModule::OpenTableEditorWindow));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(EditorCommands.ToSharedRef());

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		ZombieSpawnTabName,
		FOnSpawnTab::CreateRaw(this, &FDdProjectEditorModule::SpawnZombieSpawnTab))
		.SetDisplayName(LOCTEXT("ZombieSpawnTabTitle", "Zombie Spawn"))
		.SetTooltipText(LOCTEXT("ZombieSpawnTabTooltip", "Open the zombie spawn window."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TableEditorTabName,
		FOnSpawnTab::CreateRaw(this, &FDdProjectEditorModule::SpawnTableEditorTab))
		.SetDisplayName(LOCTEXT("TableEditorTabTitle", "Table Editor"))
		.SetTooltipText(LOCTEXT("TableEditorTabTooltip", "Open the table editor window."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());
}

void FDdProjectEditorModule::ShutdownModule()
{
	if (IsRunningCommandlet())
	{
		return;
	}

	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetGlobalLevelEditorActions()->UnmapAction(FDdProjectEditorCommands::Get().OpenZombieSpawnWindow);
		LevelEditorModule.GetGlobalLevelEditorActions()->UnmapAction(FDdProjectEditorCommands::Get().OpenTableEditorWindow);
	}

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ZombieSpawnTabName);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TableEditorTabName);

	EditorCommands.Reset();
	FDdProjectEditorCommands::Unregister();
}

void FDdProjectEditorModule::OpenZombieSpawnWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FTabId(ZombieSpawnTabName));
}

void FDdProjectEditorModule::OpenTableEditorWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TableEditorTabName));
}

void FDdProjectEditorModule::SpawnZombieFromZombieSpawnPanel()
{
	UWorld* World = GetTargetWorld();
	if (World == nullptr)
	{
		ShowNotification(LOCTEXT("NoWorld", "No target world is available for zombie spawning."));
		return;
	}

	ADdTestManager* TestManager = ADdTestManager::FindOrCreate(World);
	if (TestManager == nullptr)
	{
		ShowNotification(LOCTEXT("NoTestManager", "Failed to find or create TestManager."));
		return;
	}

	ADdMonsterCharacter* SpawnedZombie = TestManager->SpawnZombie();
	if (SpawnedZombie == nullptr)
	{
		ShowNotification(LOCTEXT("SpawnZombieFailed", "Failed to spawn a zombie. Make sure a player character exists in the current world."));
		return;
	}

	if (GEditor != nullptr && GEditor->PlayWorld == World)
	{
		ShowNotification(LOCTEXT("ZombieSpawnedInPlayWorld", "Zombie spawned near the player in the current play world."));
		return;
	}

	ShowNotification(LOCTEXT("ZombieSpawnedInEditor", "Zombie spawned in the editor world."));
}

TSharedRef<SDockTab> FDdProjectEditorModule::SpawnZombieSpawnTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("ZombieSpawnTabLabel", "Zombie Spawn"))
		[
			SNew(SDdZombieSpawnPanel)
			.OnSpawnZombie(FSimpleDelegate::CreateRaw(this, &FDdProjectEditorModule::SpawnZombieFromZombieSpawnPanel))
		];
}

TSharedRef<SDockTab> FDdProjectEditorModule::SpawnTableEditorTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("TableEditorTabLabel", "Table Editor"))
		[
			SNew(SDdTableEditorPanel)
		];
}

UWorld* FDdProjectEditorModule::GetTargetWorld() const
{
	if (GEditor == nullptr)
	{
		return nullptr;
	}

	if (GEditor->PlayWorld != nullptr)
	{
		return GEditor->PlayWorld;
	}

	return GEditor->GetEditorWorldContext().World();
}

void FDdProjectEditorModule::ShowNotification(const FText& Message) const
{
	FNotificationInfo NotificationInfo(Message);
	NotificationInfo.bFireAndForget = true;
	NotificationInfo.ExpireDuration = 3.0f;

	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
}

#undef LOCTEXT_NAMESPACE
