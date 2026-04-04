#pragma once

#include "GameFramework/HUD.h"
#include "DdGameplayHud.generated.h"

UCLASS()
class DDPROJECT_API ADdGameplayHud : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
};
