#include "DdScreenFadeWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"

float UDdScreenFadeWidget::GetFadeAlpha() const
{
	return CurrentFadeAlpha;
}

void UDdScreenFadeWidget::SetFadeAlpha(float InAlpha)
{
	CurrentFadeAlpha = FMath::Clamp(InAlpha, 0.0f, 1.0f);
	SetRenderOpacity(CurrentFadeAlpha);
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UDdScreenFadeWidget::StartFade(float FromAlpha, float ToAlpha, float Duration)
{
	FadeElapsedTime = 0.0f;
	FadeDuration = FMath::Max(Duration, 0.0f);
	FadeStartAlpha = FMath::Clamp(FromAlpha, 0.0f, 1.0f);
	FadeTargetAlpha = FMath::Clamp(ToAlpha, 0.0f, 1.0f);
	bIsFading = FadeDuration > KINDA_SMALL_NUMBER;

	SetFadeAlpha(FadeStartAlpha);

	if (!bIsFading)
	{
		SetFadeAlpha(FadeTargetAlpha);
		OnFadeFinished.Broadcast();
	}
}

void UDdScreenFadeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	EnsureFadeImage();
	SetIsEnabled(false);
	SetFadeAlpha(CurrentFadeAlpha);
}

void UDdScreenFadeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsFading)
	{
		return;
	}

	FadeElapsedTime += InDeltaTime;

	const float FadeProgress = FadeDuration <= KINDA_SMALL_NUMBER
		? 1.0f
		: FMath::Clamp(FadeElapsedTime / FadeDuration, 0.0f, 1.0f);

	SetFadeAlpha(FMath::Lerp(FadeStartAlpha, FadeTargetAlpha, FadeProgress));

	if (FadeProgress < 1.0f)
	{
		return;
	}

	bIsFading = false;
	OnFadeFinished.Broadcast();
}

void UDdScreenFadeWidget::EnsureFadeImage()
{
	if (WidgetTree == nullptr)
	{
		return;
	}

	if (FadeImage == nullptr)
	{
		FadeImage = Cast<UImage>(WidgetTree->FindWidget(TEXT("FadeImage")));
	}

	if (WidgetTree->RootWidget == nullptr)
	{
		UCanvasPanel* RootPanel = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
		WidgetTree->RootWidget = RootPanel;

		FadeImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("FadeImage"));
		FadeImage->SetColorAndOpacity(FLinearColor::Black);

		if (UCanvasPanelSlot* FadeImageSlot = RootPanel->AddChildToCanvas(FadeImage))
		{
			FadeImageSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
			FadeImageSlot->SetOffsets(FMargin(0.0f));
		}

		return;
	}

	if (FadeImage == nullptr)
	{
		if (UCanvasPanel* RootPanel = Cast<UCanvasPanel>(WidgetTree->RootWidget))
		{
			FadeImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("FadeImage"));
			FadeImage->SetColorAndOpacity(FLinearColor::Black);

			if (UCanvasPanelSlot* FadeImageSlot = RootPanel->AddChildToCanvas(FadeImage))
			{
				FadeImageSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
				FadeImageSlot->SetOffsets(FMargin(0.0f));
			}
		}
	}
	else
	{
		FadeImage->SetColorAndOpacity(FLinearColor::Black);
	}
}
