#pragma once

#include "Blueprint/UserWidget.h"
#include "DdTitleScreenWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnStartGameRequested);

UCLASS()
class DDPROJECT_API UDdTitleScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnStartGameRequested OnStartGameRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleStartButtonClicked();

	UPROPERTY(meta=(BindWidgetOptional))
	class UButton* StartButton;
};
