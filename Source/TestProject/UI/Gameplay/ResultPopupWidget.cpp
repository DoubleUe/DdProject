#include "ResultPopupWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ContentWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UResultPopupWidget::ShowPopup()
{
	SetVisibility(ESlateVisibility::Visible);
	SetIsEnabled(true);
}

void UResultPopupWidget::HidePopup()
{
	SetIsEnabled(false);
	SetVisibility(ESlateVisibility::Collapsed);
}

void UResultPopupWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ConfirmButton != nullptr)
	{
		ConfirmButton->OnClicked.RemoveAll(this);
		ConfirmButton->OnClicked.AddDynamic(this, &UResultPopupWidget::HandleConfirmButtonClicked);
	}
}

void UResultPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureWidgetTree();
	EnsureConfirmButtonContent();

	if (ConfirmButton != nullptr)
	{
		ConfirmButton->OnClicked.RemoveAll(this);
		ConfirmButton->OnClicked.AddDynamic(this, &UResultPopupWidget::HandleConfirmButtonClicked);
	}

	HidePopup();
}

void UResultPopupWidget::HandleConfirmButtonClicked()
{
	HidePopup();
	OnClosed.Broadcast();
}

void UResultPopupWidget::EnsureWidgetTree()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	if (RootCanvas == nullptr)
	{
		RootCanvas = Cast<UCanvasPanel>(WidgetTree->FindWidget(TEXT("RootCanvas")));
	}

	if (SuccessImage == nullptr)
	{
		SuccessImage = Cast<UImage>(WidgetTree->FindWidget(TEXT("SuccessImage")));
	}

	if (ConfirmButton == nullptr)
	{
		ConfirmButton = Cast<UButton>(WidgetTree->FindWidget(TEXT("ConfirmButton")));
	}

	if (ConfirmButtonText == nullptr)
	{
		ConfirmButtonText = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("ConfirmButtonText")));
	}

	if (WidgetTree->RootWidget == nullptr)
	{
		RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootCanvas;
	}
	else if (RootCanvas == nullptr)
	{
		RootCanvas = Cast<UCanvasPanel>(WidgetTree->RootWidget);
	}

	if (RootCanvas == nullptr)
	{
		return;
	}

	if (SuccessImage == nullptr)
	{
		SuccessImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("SuccessImage"));
		SuccessImage->SetColorAndOpacity(FLinearColor::White);

		if (UCanvasPanelSlot* ImageSlot = RootCanvas->AddChildToCanvas(SuccessImage))
		{
			ImageSlot->SetAnchors(FAnchors(0.5f, 1.0f / 3.0f, 0.5f, 1.0f / 3.0f));
			ImageSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			ImageSlot->SetOffsets(FMargin(0.0f, 0.0f, SuccessImageSize.X, SuccessImageSize.Y));
		}
	}

	if (ConfirmButton == nullptr)
	{
		ConfirmButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("ConfirmButton"));
		ConfirmButton->SetBackgroundColor(FLinearColor::White);

		if (UCanvasPanelSlot* ButtonSlot = RootCanvas->AddChildToCanvas(ConfirmButton))
		{
			ButtonSlot->SetAnchors(FAnchors(0.5f, 2.0f / 3.0f, 0.5f, 2.0f / 3.0f));
			ButtonSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			ButtonSlot->SetOffsets(FMargin(0.0f, 0.0f, ConfirmButtonSize.X, ConfirmButtonSize.Y));
		}
	}
}

void UResultPopupWidget::EnsureConfirmButtonContent()
{
	if (ConfirmButton == nullptr)
	{
		return;
	}

	if (ConfirmButtonText == nullptr)
	{
		ConfirmButtonText = Cast<UTextBlock>(ConfirmButton->GetContent());
	}

	if (ConfirmButtonText == nullptr)
	{
		ConfirmButtonText = WidgetTree != nullptr
			? WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ConfirmButtonText"))
			: NewObject<UTextBlock>(this, TEXT("ConfirmButtonText"));

		if (ConfirmButtonText != nullptr)
		{
			ConfirmButton->SetContent(ConfirmButtonText);
		}
	}

	if (ConfirmButtonText != nullptr)
	{
		ConfirmButtonText->SetText(ConfirmButtonLabel);
		ConfirmButtonText->SetJustification(ETextJustify::Center);
		ConfirmButtonText->SetColorAndOpacity(FSlateColor(FLinearColor::Black));
	}
}
