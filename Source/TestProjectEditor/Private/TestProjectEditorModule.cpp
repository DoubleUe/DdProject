#include "TestProjectEditorModule.h"

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
#include "Test/ZombiePatrolTestCharacter.h"
#include "TestProjectEditorCommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/STestToolsPanel.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FTestProjectEditorModule"

const FName FTestProjectEditorModule::TestToolsTabName(TEXT("TestProjectEditor_TestTools"));

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

IMPLEMENT_MODULE(FTestProjectEditorModule, TestProjectEditor)

void FTestProjectEditorModule::StartupModule()
{
	FTestProjectEditorCommands::Register();

	EditorCommands = MakeShared<FUICommandList>();
	EditorCommands->MapAction(
		FTestProjectEditorCommands::Get().OpenTestToolsWindow,
		FExecuteAction::CreateRaw(this, &FTestProjectEditorModule::OpenTestToolsWindow));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(EditorCommands.ToSharedRef());

	ShortcutInputProcessor = MakeShared<FTestToolsShortcutInputProcessor>([this]()
	{
		OpenTestToolsWindow();
	});

	FSlateApplication::Get().RegisterInputPreProcessor(ShortcutInputProcessor);

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TestToolsTabName,
		FOnSpawnTab::CreateRaw(this, &FTestProjectEditorModule::SpawnTestToolsTab))
		.SetDisplayName(LOCTEXT("TestToolsTabTitle", "Test Tools"))
		.SetTooltipText(LOCTEXT("TestToolsTabTooltip", "Open the Test Tools window."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory());
}

void FTestProjectEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("LevelEditor"))
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::GetModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetGlobalLevelEditorActions()->UnmapAction(FTestProjectEditorCommands::Get().OpenTestToolsWindow);
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
	FTestProjectEditorCommands::Unregister();
}

void FTestProjectEditorModule::OpenTestToolsWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FTabId(TestToolsTabName));
}

void FTestProjectEditorModule::RunZombiePatrolTest()
{
	UWorld* World = GetTargetWorld();
	if (World == nullptr)
	{
		ShowNotification(LOCTEXT("NoWorld", "No target world is available for the test."));
		return;
	}

	ATestManager* TestManager = ATestManager::FindOrCreate(World);
	if (TestManager == nullptr)
	{
		ShowNotification(LOCTEXT("NoTestManager", "Failed to find or create TestManager."));
		return;
	}

	const bool bWasRunning = TestManager->IsZombiePatrolTestRunning();
	AZombiePatrolTestCharacter* SpawnedZombie = TestManager->RunZombiePatrolTest();
	if (SpawnedZombie == nullptr)
	{
		ShowNotification(LOCTEXT("ZombiePatrolFailed", "Zombie Patrol Test failed to start."));
		return;
	}

	if (bWasRunning)
	{
		ShowNotification(LOCTEXT("ZombiePatrolAlreadyRunning", "Zombie Patrol Test is already running."));
		return;
	}

	if (GEditor != nullptr && GEditor->PlayWorld == World)
	{
		ShowNotification(LOCTEXT("ZombiePatrolStartedInPlayWorld", "Zombie Patrol Test started in the current play world."));
		return;
	}

	ShowNotification(LOCTEXT("ZombiePatrolSpawnedInEditor", "Zombie Patrol Test actor spawned in the editor world. Use PIE or Simulate to see movement."));
}

TSharedRef<SDockTab> FTestProjectEditorModule::SpawnTestToolsTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("TestToolsTabLabel", "Test Tools"))
		[
			SNew(STestToolsPanel)
			.OnRunZombiePatrolTest(FSimpleDelegate::CreateRaw(this, &FTestProjectEditorModule::RunZombiePatrolTest))
		];
}

UWorld* FTestProjectEditorModule::GetTargetWorld() const
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

void FTestProjectEditorModule::ShowNotification(const FText& Message) const
{
	FNotificationInfo NotificationInfo(Message);
	NotificationInfo.bFireAndForget = true;
	NotificationInfo.ExpireDuration = 3.0f;

	FSlateNotificationManager::Get().AddNotification(NotificationInfo);
}

#undef LOCTEXT_NAMESPACE
