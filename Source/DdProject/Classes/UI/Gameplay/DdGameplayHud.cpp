#include "DdGameplayHud.h"

#include "../../PlayerController/Components/DdLocalPlayerGridComponent.h"
#include "GameFramework/PlayerController.h"

void ADdGameplayHud::DrawHUD()
{
	Super::DrawHUD();

	if (Canvas == nullptr)
	{
		return;
	}

	const APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController == nullptr)
	{
		return;
	}

	if (const UDdLocalPlayerGridComponent* LocalPlayerGridComponent = PlayerController->FindComponentByClass<UDdLocalPlayerGridComponent>())
	{
		LocalPlayerGridComponent->DrawGrid(Canvas);
	}
}
