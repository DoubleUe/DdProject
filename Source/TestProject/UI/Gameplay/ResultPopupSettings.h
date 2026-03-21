#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ResultPopupSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Result Popup Settings"))
class TESTPROJECT_API UResultPopupSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="Result Popup")
	TSoftClassPtr<class UResultPopupWidget> ResultPopupWidgetClass;
};
