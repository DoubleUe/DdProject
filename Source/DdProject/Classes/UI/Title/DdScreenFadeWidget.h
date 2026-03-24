#pragma once

#include "Blueprint/UserWidget.h"
#include "DdScreenFadeWidget.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnScreenFadeFinished);

UCLASS()
class DDPROJECT_API UDdScreenFadeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FOnScreenFadeFinished OnFadeFinished;

	float GetFadeAlpha() const;
	void SetFadeAlpha(float InAlpha);
	void StartFade(float FromAlpha, float ToAlpha, float Duration);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void EnsureFadeImage();

	UPROPERTY(meta=(BindWidgetOptional))
	class UImage* FadeImage;

	float CurrentFadeAlpha = 1.0f;
	float FadeElapsedTime = 0.0f;
	float FadeDuration = 0.0f;
	float FadeStartAlpha = 1.0f;
	float FadeTargetAlpha = 0.0f;
	bool bIsFading = false;
};
