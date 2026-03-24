#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class STestToolsPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STestToolsPanel)
	{
	}
		SLATE_EVENT(FSimpleDelegate, OnSpawnZombie)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply HandleSpawnZombieClicked();

	FSimpleDelegate OnSpawnZombie;
};
