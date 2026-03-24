#include "DdTitleScreenWidget.h"

#include "Components/Button.h"

void UDdTitleScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (StartButton != nullptr)
	{
		StartButton->OnClicked.AddDynamic(this, &UDdTitleScreenWidget::HandleStartButtonClicked);
	}
}

void UDdTitleScreenWidget::HandleStartButtonClicked()
{
	OnStartGameRequested.Broadcast();
}
