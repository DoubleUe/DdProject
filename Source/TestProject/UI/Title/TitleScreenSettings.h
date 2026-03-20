#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "TitleScreenSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Title Screen Settings"))
class TESTPROJECT_API UTitleScreenSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UTitleScreenSettings();

	UPROPERTY(Config, EditAnywhere, Category="Title Screen")
	TSoftClassPtr<class UTitleScreenWidget> TitleScreenWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen")
	TSoftClassPtr<class UScreenFadeWidget> ScreenFadeWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen")
	FName TitleLevelName;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen")
	FName GameLevelName;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen", meta=(ClampMin="0.0"))
	float FadeOutDuration = 5;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen", meta=(ClampMin="0.0"))
	float FadeInDuration;
};
