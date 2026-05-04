#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Lab_Types.h"
#include "Lab_GameMode.generated.h"

/**
 * ALab_GameMode
 *
 * Only exists on the server. Never runs on clients.
 *
 * Round rotation overview:
 *   - Three players each take one turn as monster.
 *   - Monster order by join index: {2, 0, 1} (3rd joined first, then 1st, then 2nd).
 *   - When all survivors are caught the round ends: timer pauses, result is recorded,
 *     a 3-second countdown fires, then the next round begins (or results are shown).
 *
 * Spawn point tags (set on Player Start actors in the level):
 *   - SurvivorSpawn (needs 2 actors)
 *   - MonsterSpawn  (needs 1 actor)
 *
 * Create BP_Lab_GameMode as a Blueprint child and set:
 *   - SurvivorPawnClass
 *   - MonsterPawnClass
 *   - HUDClass → BP_Lab_HUD
 */
UCLASS()
class LABRYNTH_PROJECT_API ALab_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALab_GameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// Call this from the monster character's overlap/hit logic when it tags a survivor.
	// Pass the PlayerController of the survivor who was caught.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|WinCondition")
	void NotifySurvivorCaught(APlayerController* CaughtPlayer);

	// Called by the PlayerController's Server RPC when a player presses E to ready up.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|Rounds")
	void NotifyPlayerReady();

	// Call this from the exit trigger in the maze level when a survivor overlaps it.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|WinCondition")
	void NotifySurvivorEscaped();

protected:
	// ── Set these in BP_Lab_GameMode's Class Defaults ─────────────────────────

	// Pawn spawned for each survivor player.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	TSubclassOf<APawn> SurvivorPawnClass;

	// Pawn spawned for the monster player.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	TSubclassOf<APawn> MonsterPawnClass;

	// Actor tag used to identify survivor spawn points placed in the maze level.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	FName SurvivorSpawnTag = TEXT("SurvivorSpawn");

	// Actor tag used to identify the monster spawn point(s).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	FName MonsterSpawnTag = TEXT("MonsterSpawn");

	// How many players must connect before the game starts (survivors = MaxPlayers-1, monster = 1).
	// Set this in BP_Lab_GameMode to match the lobby player count.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	int32 MaxPlayers = 3;

	// Seconds to pause between the end of one round and the start of the next.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	float RoundTransitionDelay = 3.f;

	// ── Optional Blueprint overrides ──────────────────────────────────────────

	// Called on the server when a round finishes (all survivors caught).
	// Override in BP_Lab_GameMode to show a round-end animation or sound.
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "Game|Rounds")
	void OnRoundComplete(int32 RoundNumber, const FString& MonsterName, float TimeSeconds);
	virtual void OnRoundComplete_Implementation(int32 RoundNumber, const FString& MonsterName, float TimeSeconds);

	// Called on the server after all rounds have finished.
	// Override in BP_Lab_GameMode to do any final cleanup before ShowingResults.
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "Game|Rounds")
	void OnAllRoundsComplete();
	virtual void OnAllRoundsComplete_Implementation();

private:
	// All connected controllers in join order (index 0 = first to join).
	TArray<APlayerController*> AllPlayers;

	int32 ConnectedPlayerCount = 0;
	int32 CaughtSurvivorCount  = 0;
	int32 EscapedSurvivorCount = 0;

	// Which round we're currently running (0-based index into MonsterOrder).
	int32 CurrentRoundIdx = 0;

	// How many players have readied up this round (server-only counter).
	int32 ReadyPlayerCount = 0;

	FTimerHandle RoundTransitionHandle;

	// ── Round management ──────────────────────────────────────────────────────

	// Transitions from WaitingToStart → InProgress when all players are ready.
	void StartCurrentRound();

	// Records the round result, pauses the timer, and schedules the transition.
	void FinishCurrentRound();

	// Fires after RoundTransitionDelay; starts the next round or shows results.
	void OnRoundTransitionComplete();

	// Destroys all existing pawns and respawns everyone with fresh roles.
	// MonsterPlayerIdx is the index into AllPlayers who should be the monster.
	void RespawnAllPlayers(int32 MonsterPlayerIdx);

	// ── Helpers ───────────────────────────────────────────────────────────────

	// Returns the world transform of the Nth actor with the given tag.
	FTransform FindSpawnTransform(FName Tag, int32 Index) const;

	// Spawns a pawn of PawnClass at SpawnTransform, possesses it with PC,
	// and sets the player's role in their PlayerState.
	void SpawnAndPossess(APlayerController* PC, TSubclassOf<APawn> PawnClass,
	                     const FTransform& SpawnTransform, EPlayerRole AssignedRole);

	void EvaluateWinCondition();
};
