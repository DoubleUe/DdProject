#pragma once

#include "Blueprint/UserWidget.h"
#include "TitleScreenWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnStartGameRequested);

UCLASS()
class TESTPROJECT_API UTitleScreenWidget : public UUserWidget
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
