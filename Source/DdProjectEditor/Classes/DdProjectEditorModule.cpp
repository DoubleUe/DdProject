#include "DdProjectEditorModule.h"

#include "Editor.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "InputCoreTypes.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "Test/TestManager.h"
#include "Commands/DdProjectEditorCommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/SDdTestToolsPanel.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FDdProjectEditorModule"

const FName FDdProjectEditorModule::TestToolsTabName(TEXT("DdProjectEditor_TestTools"));

namespace
{
	class FTestToolsShortcutInputProcessor final : public IInputProcessor
	{
	public:
		explicit FTestToolsShortcutInputProcessor(TFunction<void()> InOnShortcutPressed)
			: OnShortcutPressed(MoveTemp(InOnShortcutPressed))
		{
		}

		virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
		{
		}

		virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
		{
			if (InKeyEvent.IsRepeat())
			{
				return false;
			}

			if (InKeyEvent.GetKey() != EKeys::RightBracket)
			{
				return false;
			}

			if (InKeyEvent.IsAltDown() || InKeyEvent.IsControlDown() || InKeyEvent.IsCommandDown())
			{
				return false;
			}

			const TSharedPtr<SWidget> FocusedWidget = SlateApp.GetKeyboardFocusedWidget();
			if (FocusedWidget.IsValid())
			{
				const FName FocusedWidgetType = FocusedWidget->GetType();
				if (FocusedWidgetType == TEXT("SEditableText")
					|| FocusedWidgetType == TEXT("SEditableTextBox")
					|| FocusedWidgetType == TEXT("SMultiLineEditableText")
					|| FocusedWidgetType == TEXT("SMultiLineEditableTextBox"))
				{
					return false;
				}
			}

			if (OnShortcutPressed)
			{
				OnShortcutPressed();
				return true;
			}

			return false;
		}

		virtual const TCHAR* GetDebugName() const override
		{
			return TEXT("FTestToolsShortcutInputProcessor");
		}

	private:
		TFunction<void()> OnShortcutPressed;
	};
}

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
		FDdProjectEditorCommands::Get().OpenTestToolsWindow,
		FExecuteAction::CreateRaw(this, &FDdProjectEditorModule::OpenTestToolsWindow));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(EditorCommands.ToSharedRef());

	ShortcutInputProcessor = MakeShared<FTestToolsShortcutInputProcessor>([this]()
	{
		OpenTestToolsWindow();
	});

	FSlateApplication::Get().RegisterInputPreProcessor(ShortcutInputProcessor);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TestToolsTabName,
		FOnSpawnTab::CreateRaw(this, &FDdProjectEditorModule::SpawnTestToolsTab))
		.SetDisplayName(LOCTEXT("TestToolsTabTitle", "Zombie Spawn Tools"))
		.SetTooltipText(LOCTEXT("TestToolsTabTooltip", "Open the zombie spawn tools window."))
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
		LevelEditorModule.GetGlobalLevelEditorActions()->UnmapAction(FDdProjectEditorCommands::Get().OpenTestToolsWindow);
	}

	if (FSlateApplication::IsInitialized())
	{
		if (ShortcutInputProcessor.IsValid())
		{
			FSlateApplication::Get().UnregisterInputPreProcessor(ShortcutInputProcessor);
		}

		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TestToolsTabName);
	}

	ShortcutInputProcessor.Reset();
	EditorCommands.Reset();
	FDdProjectEditorCommands::Unregister();
}

void FDdProjectEditorModule::OpenTestToolsWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TestToolsTabName));
}

void FDdProjectEditorModule::SpawnZombieFromTools()
{
	UWorld* World = GetTargetWorld();
	if (World == nullptr)
	{
		ShowNotification(LOCTEXT("NoWorld", "No target world is available for zombie spawning."));
		return;
	}

	ATestManager* TestManager = ATestManager::FindOrCreate(World);
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

TSharedRef<SDockTab> FDdProjectEditorModule::SpawnTestToolsTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("TestToolsTabLabel", "Zombie Spawn Tools"))
		[
			SNew(SDdTestToolsPanel)
			.OnSpawnZombie(FSimpleDelegate::CreateRaw(this, &FDdProjectEditorModule::SpawnZombieFromTools))
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
