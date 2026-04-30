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

// Fired on every machine when this player's display name is set or changes.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisplayNameChanged, FString, NewName);

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

	// True once this player has pressed E to confirm they are ready for the round to start.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player|Status")
	bool bIsReady = false;

	// The username this player typed in the main menu. Set by Server_SetDisplayName
	// in the PlayerController. Replicated so all machines can read it for nameplates.
	UPROPERTY(ReplicatedUsing = OnRep_DisplayName, BlueprintReadOnly, Category = "Player|Identity")
	FString DisplayName;

	// ── Server-only setters ───────────────────────────────────────────────────

	// Called by the GameMode in PostLogin to assign this player's role.
	// BlueprintAuthorityOnly: calling this from a client Blueprint does nothing.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Role")
	void SetPlayerRole(EPlayerRole NewRole);

	// Called by the GameMode or character when the monster makes contact.
	// Safe to call multiple times — ignores calls after the first.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Status")
	void SetCaught();

	// Resets caught state at the start of a new round so the player can be caught again.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Status")
	void ResetCaught();

	// Marks this player as ready for the round to begin.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Status")
	void SetReady();

	// Clears the ready flag between rounds.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Status")
	void ResetReady();

	// Called by the PlayerController's Server RPC when the player confirms their username.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Player|Identity")
	void SetDisplayName(const FString& Name);

	// ── Blueprint-assignable events ───────────────────────────────────────────

	// Bind to this in WBP_HUD or the character Blueprint to react when caught.
	UPROPERTY(BlueprintAssignable, Category = "Player|Events")
	FOnSurvivorCaught OnSurvivorCaught;

	// Bind to this in the character Blueprint to set up role-specific UI/camera.
	UPROPERTY(BlueprintAssignable, Category = "Player|Events")
	FOnRoleAssigned OnRoleAssigned;

	// Fires on every machine when DisplayName is set or changed.
	UPROPERTY(BlueprintAssignable, Category = "Player|Events")
	FOnDisplayNameChanged OnDisplayNameChanged;

private:
	UFUNCTION()
	void OnRep_PlayerRole();

	UFUNCTION()
	void OnRep_bIsCaught();

	UFUNCTION()
	void OnRep_DisplayName();
};
