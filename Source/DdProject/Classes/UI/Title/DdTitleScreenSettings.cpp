#include "DdTitleScreenSettings.h"

UDdTitleScreenSettings::UDdTitleScreenSettings()
{
	TitleLevelName = TEXT("/Game/Maps/TitleLevel");
	GameLevelName = TEXT("/Game/Maps/GameLevel");
	FadeOutDuration = 5.0f;
	FadeInDuration = 1.0f;
}
