#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Lab_Types.h"
#include "Lab_GameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnConnectedCountChanged, int32, NewCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundResultsUpdated);

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

	virtual void Tick(float DeltaTime) override;
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

	// Seconds elapsed in the current round. Server increments this; clients read it.
	// Use GetTimerText() in WBP_HUD for a formatted MM:SS.cs string.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game|Timer")
	float RoundElapsedTime = 0.f;

	// Which round we are currently on (1-indexed, max 3).
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game|Rounds")
	int32 CurrentRound = 1;

	// Leaderboard entries, one per completed round. Populated server-side as rounds finish.
	// Broadcast via OnRoundResultsUpdated when new results arrive on clients.
	UPROPERTY(ReplicatedUsing = OnRep_RoundResults, BlueprintReadOnly, Category = "Game|Rounds")
	TArray<FRoundResult> RoundResults;

	// ── Server-only setters ───────────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game")
	void SetGamePhase(EGamePhase NewPhase);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game")
	void SetConnectedPlayerCount(int32 Count);

	// Increments the caught count and fires OnRep. Called by the GameMode when a survivor is tagged.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game")
	void NotifySurvivorCaught();

	// Starts ticking the round timer. Call at the beginning of each round.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|Timer")
	void StartRoundTimer();

	// Freezes the round timer. Call when the last survivor is caught.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|Timer")
	void PauseRoundTimer();

	// Records a completed round result. Results are replicated to all clients.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|Rounds")
	void AddRoundResult(const FString& PlayerName, float TimeSeconds);

	// Resets per-round counters and the timer for the start of a new round.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|Rounds")
	void ResetForNewRound(int32 NewRoundNumber);

	// ── Blueprint-assignable events ───────────────────────────────────────────

	// Bind in WBP_HUD or anywhere that needs to react to phase changes.
	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FOnGamePhaseChanged OnGamePhaseChanged;

	// Bind in the lobby UI to update a "Players Connected: X/3" counter.
	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FOnConnectedCountChanged OnConnectedCountChanged;

	// Fires on all clients when RoundResults gains a new entry (or when ShowingResults begins).
	// Bind in WBP_HUD to trigger the leaderboard panel.
	UPROPERTY(BlueprintAssignable, Category = "Game|Events")
	FOnRoundResultsUpdated OnRoundResultsUpdated;

private:
	// Server-only; not replicated. Clients infer timer state from GamePhase.
	bool bTimerActive = false;

	UFUNCTION()
	void OnRep_GamePhase();

	UFUNCTION()
	void OnRep_ConnectedPlayerCount();

	UFUNCTION()
	void OnRep_CaughtSurvivorCount();

	UFUNCTION()
	void OnRep_RoundResults();
};
