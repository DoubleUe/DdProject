#pragma once

#include "Blueprint/UserWidget.h"
#include "DdTitleScreenWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnSingleGameRequested);
DECLARE_MULTICAST_DELEGATE(FOnHostGameRequested);
DECLARE_MULTICAST_DELEGATE(FOnJoinGameRequested);

UCLASS()
class DDPROJECT_API UDdTitleScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnSingleGameRequested OnSingleGameRequested;
	FOnHostGameRequested OnHostGameRequested;
	FOnJoinGameRequested OnJoinGameRequested;

protected:
	virtual void NativeOnInitialized() override;

private:
	UFUNCTION()
	void HandleSingleButtonClicked();

	UFUNCTION()
	void HandleHostButtonClicked();

	UFUNCTION()
	void HandleJoinButtonClicked();
	
	UPROPERTY(meta=(BindWidgetOptional))
	class UButton* SingleButton;

	UPROPERTY(meta=(BindWidgetOptional))
	class UButton* HostButton;

	UPROPERTY(meta=(BindWidgetOptional))
	class UButton* JoinButton;
};
