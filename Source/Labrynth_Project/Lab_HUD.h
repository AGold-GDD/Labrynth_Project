#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Lab_HUDWidget.h"
#include "Lab_HUD.generated.h"

/**
 * ALab_HUD
 *
 * Spawns and displays the in-game HUD widget for each local player.
 *
 * Setup:
 *   1. Create WBP_HUD as a Blueprint child of ULab_HUDWidget in Content/MultiplayerStuff/.
 *   2. Create BP_Lab_HUD as a Blueprint child of ALab_HUD.
 *   3. In BP_Lab_HUD's Class Defaults set HUDWidgetClass = WBP_HUD.
 *   4. In BP_Lab_GameMode's Class Defaults set HUDClass = BP_Lab_HUD.
 *      (ALab_GameMode already sets HUDClass = ALab_HUD::StaticClass() as a default,
 *       so the Blueprint child overrides it automatically.)
 */
UCLASS()
class LABRYNTH_PROJECT_API ALab_HUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// Set this to WBP_HUD (Blueprint child of ULab_HUDWidget) in BP_Lab_HUD's Class Defaults.
	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<ULab_HUDWidget> HUDWidgetClass;

	// The live widget instance. Exposed so Blueprint can reach it if needed.
	UPROPERTY(BlueprintReadOnly, Category = "HUD")
	TObjectPtr<ULab_HUDWidget> HUDWidget;
};
