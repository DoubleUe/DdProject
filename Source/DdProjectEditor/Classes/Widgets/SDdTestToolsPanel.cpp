#include "SDdTestToolsPanel.h"

#include "Input/Reply.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SDdTestToolsPanel"

void SDdTestToolsPanel::Construct(const FArguments& InArgs)
{
	OnSpawnZombie = InArgs._OnSpawnZombie;

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
				.Text(LOCTEXT("Title", "Zombie Spawn Tools"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("Description", "Spawn zombies into the current world. Spawned and placed zombies will automatically chase the player during gameplay."))
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
				.Text(LOCTEXT("SpawnZombieLabel", "Spawn Zombie"))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("SpawnZombieDescription", "Find or create TestManager and spawn a zombie at a random position within 10 meters of the player."))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 12.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("SpawnZombieButton", "Spawn Zombie"))
				.OnClicked(this, &SDdTestToolsPanel::HandleSpawnZombieClicked)
			]
		]
	];
}

FReply SDdTestToolsPanel::HandleSpawnZombieClicked()
{
	if (OnSpawnZombie.IsBound())
	{
		OnSpawnZombie.Execute();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
