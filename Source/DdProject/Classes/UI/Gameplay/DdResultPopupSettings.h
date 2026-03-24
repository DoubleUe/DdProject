#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DdResultPopupSettings.generated.h"

UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Result Popup Settings"))
class DDPROJECT_API UDdResultPopupSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category="Result Popup")
	TSoftClassPtr<class UDdResultPopupWidget> ResultPopupWidgetClass;
};
