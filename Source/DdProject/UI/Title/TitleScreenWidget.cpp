#include "TitleScreenWidget.h"

#include "Components/Button.h"

void UTitleScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (StartButton != nullptr)
	{
		StartButton->OnClicked.AddDynamic(this, &UTitleScreenWidget::HandleStartButtonClicked);
	}
}

void UTitleScreenWidget::HandleStartButtonClicked()
{
	OnStartGameRequested.Broadcast();
}
