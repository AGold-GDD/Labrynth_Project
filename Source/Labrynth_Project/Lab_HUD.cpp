#include "Lab_HUD.h"
#include "GameFramework/PlayerController.h"

void ALab_HUD::BeginPlay()
{
	Super::BeginPlay();

	// Only create the widget on the owning local player's machine.
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !PC->IsLocalController()) return;
	if (!HUDWidgetClass) return;

	HUDWidget = CreateWidget<ULab_HUDWidget>(PC, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
	}
}
