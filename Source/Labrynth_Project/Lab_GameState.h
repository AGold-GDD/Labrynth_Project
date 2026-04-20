#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Lab_Types.h"
#include "Lab_GameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectedCountChanged, int32, NewCount);

/**
 * ALab_GameState
 *
 * Exists on every machine (server spawns it; clients receive a replicated copy).
 * Use it to read shared game state from any Blueprint: HUDs, character logic, etc.
 *
 * The server writes state through the setter functions (BlueprintAuthorityOnly).
 * Clients react through the BlueprintAssignable delegates, which fire automatically
 * via RepNotify whenever a replicated value changes.
 *
 * How to read from Blueprint:
 *   Get Game State > Cast to BP_Lab_GameState > read GamePhase / ConnectedPlayerCount
 *   Or bind to OnGamePhaseChanged to react the instant the phase updates.
 */
UCLASS()
class LABRYNTH_PROJECT_API ALab_GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALab_GameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ── Replicated state ──────────────────────────────────────────────────────

	// Current flow phase. Changes drive win/lose screens on all clients.
	UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "Game")
	EGamePhase GamePhase = EGamePhase::WaitingForPlayers;

	// How many players have joined so far. Useful for a "waiting for players" lobby UI.
	UPROPERTY(ReplicatedUsing = OnRep_ConnectedPlayerCount, BlueprintReadOnly, Category = "Game")
	int32 ConnectedPlayerCount = 0;

	// How many survivors have been caught. GameMode checks this for the win condition.
	UPROPERTY(ReplicatedUsing = OnRep_CaughtSurvivorCount, BlueprintReadOnly, Category = "Game")
	int32 CaughtSurvivorCount = 0;

	// ── Server-only setters ───────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game")
	void SetGamePhase(EGamePhase NewPhase);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game")
	void SetConnectedPlayerCount(int32 Count);

	// Increments the caught count and fires OnRep. Called by the GameMode when a survivor is tagged.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game")
	void NotifySurvivorCaught();

	// ── Blueprint-assignable events ───────────────────────────────────────────

	// Bind in WBP_HUD or anywhere that needs to react to phase changes.
	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FOnGamePhaseChanged OnGamePhaseChanged;

	// Bind in the lobby UI to update a "Players Connected: X/3" counter.
	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FOnConnectedCountChanged OnConnectedCountChanged;

private:
	UFUNCTION()
	void OnRep_GamePhase();

	UFUNCTION()
	void OnRep_ConnectedPlayerCount();

	UFUNCTION()
	void OnRep_CaughtSurvivorCount();
};
