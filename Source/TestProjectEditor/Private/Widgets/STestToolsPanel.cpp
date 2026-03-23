#include "Widgets/STestToolsPanel.h"

#include "Input/Reply.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "STestToolsPanel"

void STestToolsPanel::Construct(const FArguments& InArgs)
{
	OnRunZombiePatrolTest = InArgs._OnRunZombiePatrolTest;

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(16.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "Test Tools"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("Description", "Temporary entry points for runtime test flows. Use PIE or Simulate to observe character movement tests."))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 16.0f, 0.0f, 16.0f)
			[
				SNew(SSeparator)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ZombiePatrolLabel", "Zombie Patrol Test"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("ZombiePatrolDescription", "Find or create TestManager in the current world, then run the zombie forward/back patrol test."))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RunZombiePatrolButton", "Run Zombie Patrol Test"))
				.OnClicked(this, &STestToolsPanel::HandleRunZombiePatrolTestClicked)
			]
		]
	];
}

FReply STestToolsPanel::HandleRunZombiePatrolTestClicked()
{
	if (OnRunZombiePatrolTest.IsBound())
	{
		OnRunZombiePatrolTest.Execute();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
