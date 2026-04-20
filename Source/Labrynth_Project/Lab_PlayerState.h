#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Lab_Types.h"
#include "Lab_PlayerState.generated.h"

// Fired on every machine (via RepNotify) when a survivor is caught.
// Bind to this in the survivor's Blueprint to show a game-over screen.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSurvivorCaught);

// Fired on every machine when this player's role is assigned by the GameMode.
// Bind to this in the character Blueprint to set up role-specific UI or abilities.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRoleAssigned, EPlayerRole, NewRole);

/**
 * ALab_PlayerState
 *
 * Exists on every machine for every connected player.
 * Holds replicated per-player data: their assigned role and whether they were caught.
 *
 * The server writes to these properties via SetPlayerRole() and SetCaught().
 * RepNotify callbacks fire automatically on all clients when values change,
 * broadcasting Blueprint-assignable delegates so Blueprints can react without polling.
 */
UCLASS()
class LABRYNTH_PROJECT_API ALab_PlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ── Replicated state ──────────────────────────────────────────────────────

	// Assigned by the GameMode in PostLogin. Read-only on clients.
	UPROPERTY(ReplicatedUsing = OnRep_PlayerRole, BlueprintReadOnly, Category = "Player|Role")
	EPlayerRole PlayerRole = EPlayerRole::None;

	// Set true by the server when the monster tags this survivor.
	// Also used by the GameMode to count caught survivors and determine win condition.
	UPROPERTY(ReplicatedUsing = OnRep_bIsCaught, BlueprintReadOnly, Category = "Player|Status")
	bool bIsCaught = false;

	// ── Server-only setters ───────────────────────────────────────────────────

	// Called by the GameMode in PostLogin to assign this player's role.
	// BlueprintAuthorityOnly: calling this from a client Blueprint does nothing.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Role")
	void SetPlayerRole(EPlayerRole NewRole);

	// Called by the GameMode or character when the monster makes contact.
	// Safe to call multiple times — ignores calls after the first.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Status")
	void SetCaught();

	// ── Blueprint-assignable events ───────────────────────────────────────────

	// Bind to this in WBP_HUD or the character Blueprint to react when caught.
	UPROPERTY(BlueprintAssignable, Category = "Player|Events")
	FOnSurvivorCaught OnSurvivorCaught;

	// Bind to this in the character Blueprint to set up role-specific UI/camera.
	UPROPERTY(BlueprintAssignable, Category = "Player|Events")
	FOnRoleAssigned OnRoleAssigned;

private:
	UFUNCTION()
	void OnRep_PlayerRole();

	UFUNCTION()
	void OnRep_bIsCaught();
};
