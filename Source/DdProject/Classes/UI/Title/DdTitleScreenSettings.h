#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DdTitleScreenSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Title Screen Settings"))
class DDPROJECT_API UDdTitleScreenSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UDdTitleScreenSettings();

	UPROPERTY(Config, EditAnywhere, Category="Title Screen")
	TSoftClassPtr<class UDdTitleScreenWidget> TitleScreenWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen")
	TSoftClassPtr<class UDdScreenFadeWidget> ScreenFadeWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen")
	FName TitleLevelName;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen")
	FName GameLevelName;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen", meta=(ClampMin="0.0"))
	float FadeOutDuration = 5;

	UPROPERTY(Config, EditAnywhere, Category="Title Screen", meta=(ClampMin="0.0"))
	float FadeInDuration;
};
