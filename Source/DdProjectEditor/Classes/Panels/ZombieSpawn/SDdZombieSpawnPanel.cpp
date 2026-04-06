#include "Panels/ZombieSpawn/SDdZombieSpawnPanel.h"

#include "Input/Reply.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SDdZombieSpawnPanel"

void SDdZombieSpawnPanel::Construct(const FArguments& InArgs)
{
	OnSpawnZombie = InArgs._OnSpawnZombie;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SpawnZombieLabel", "Zombie Spawn"))
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
			.OnClicked(this, &SDdZombieSpawnPanel::HandleSpawnZombieClicked)
		]
	];
}

FReply SDdZombieSpawnPanel::HandleSpawnZombieClicked()
{
	if (OnSpawnZombie.IsBound())
	{
		OnSpawnZombie.Execute();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
