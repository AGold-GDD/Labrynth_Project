#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Lab_PlayerController.generated.h"

/**
 * ALab_PlayerController
 *
 * Base controller for all players (survivors and monster share this class).
 * The GameMode possesses different pawns to each player — the controller class is
 * the same because UE cannot change a player's controller class after they connect.
 *
 * This controller exposes Blueprint-callable utilities that are awkward to build
 * as pure Blueprint (input mode switching, local role helpers).
 *
 * Create BP_Lab_PlayerController as a Blueprint child of this class.
 */
UCLASS()
class LABRYNTH_PROJECT_API ALab_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// Switch input to UI-only mode: shows the cursor, enables widget clicks,
	// disables WASD/look input. Call this when showing a menu or pause screen.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void EnableUIInputMode();

	// Switch input back to game mode: hides the cursor, re-enables movement.
	// Call this when dismissing any fullscreen menu.
	UFUNCTION(BlueprintCallable, Category = "UI")
	void EnableGameInputMode();

	// Returns true if this controller's possessed pawn is the monster.
	// Useful in shared HUD/character Blueprints that need to behave differently per role.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	bool IsMonsterPlayer() const;

	// Returns true if this controller is locally controlled on this machine.
	// Use to guard client-only operations (showing UI, playing local sounds, etc.)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	bool IsLocalController_BP() const;

private:
	// Sends the local username to the server so it can write to PlayerState.
	// Called automatically in BeginPlay — no Blueprint wiring needed.
	UFUNCTION(Server, Reliable)
	void Server_SetDisplayName(const FString& Name);
};
