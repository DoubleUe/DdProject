#pragma once

#include "GameFramework/PlayerController.h"
#include "DdBasePlayerController.generated.h"

UCLASS(Abstract)
class DDPROJECT_API ADdBasePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	void InitializeSharedLocalControllerState();
	void ApplyDefaultViewportStats();
	void EnsureScreenFadeWidget();

	UPROPERTY()
	class UDdScreenFadeWidget* ScreenFadeWidget;
};
