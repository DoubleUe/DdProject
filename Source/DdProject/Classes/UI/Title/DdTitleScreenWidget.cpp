#include "DdTitleScreenWidget.h"

#include "Components/Button.h"

void UDdTitleScreenWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (SingleButton != nullptr)
	{
		SingleButton->OnClicked.AddDynamic(this, &UDdTitleScreenWidget::HandleSingleButtonClicked);
	}

	if (HostButton != nullptr)
	{
		HostButton->OnClicked.AddDynamic(this, &UDdTitleScreenWidget::HandleHostButtonClicked);
	}

	if (JoinButton != nullptr)
	{
		JoinButton->OnClicked.AddDynamic(this, &UDdTitleScreenWidget::HandleJoinButtonClicked);
	}
}

void UDdTitleScreenWidget::HandleSingleButtonClicked()
{
	OnSingleGameRequested.Broadcast();
}

void UDdTitleScreenWidget::HandleHostButtonClicked()
{
	OnHostGameRequested.Broadcast();
}

void UDdTitleScreenWidget::HandleJoinButtonClicked()
{
	OnJoinGameRequested.Broadcast();
}
