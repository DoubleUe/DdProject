#pragma once

#include "Blueprint/UserWidget.h"
#include "DdResultPopupWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnResultPopupClosed);

UCLASS()
class DDPROJECT_API UDdResultPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnResultPopupClosed OnClosed;

	void ShowPopup();
	void HidePopup();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleConfirmButtonClicked();

	void EnsureWidgetTree();
	void EnsureConfirmButtonContent();

	UPROPERTY(meta=(BindWidgetOptional))
	class UCanvasPanel* RootCanvas;

	UPROPERTY(meta=(BindWidgetOptional))
	class UImage* SuccessImage;

	UPROPERTY(meta=(BindWidgetOptional))
	class UButton* ConfirmButton;

	UPROPERTY(meta=(BindWidgetOptional))
	class UTextBlock* ConfirmButtonText;

	UPROPERTY(EditDefaultsOnly, Category="Result Popup")
	FText ConfirmButtonLabel = FText::FromString(TEXT("확인"));

	UPROPERTY(EditDefaultsOnly, Category="Result Popup", meta=(ClampMin="0.0"))
	FVector2D SuccessImageSize = FVector2D(420.0f, 140.0f);

	UPROPERTY(EditDefaultsOnly, Category="Result Popup", meta=(ClampMin="0.0"))
	FVector2D ConfirmButtonSize = FVector2D(280.0f, 72.0f);
};
