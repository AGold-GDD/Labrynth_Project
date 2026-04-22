#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LabrynthPlayerState.h"
#include "LabrynthGameMode.generated.h"

/**
 * ALabrynthGameMode
 *
 * Server-authoritative game mode for 3-player asymmetric multiplayer:
 *   Player 1 → Survivor
 *   Player 2 → Survivor
 *   Player 3 → Monster
 *
 * HOW TO USE:
 *   1. Open BP_ThirdPersonGameMode in Unreal Editor.
 *   2. In the Class Settings panel, change the Parent Class to
 *      ALabrynthGameMode (search "Labrynth Game Mode").
 *   3. In Class Defaults, set SurvivorClass and MonsterClass to your
 *      Blueprint pawns (BP_ThirdPersonCharacter and BP_NPC).
 *   4. Compile and save.
 *
 * Everything else (spawning, role assignment, phase transitions) is
 * handled here in C++. Override the BlueprintImplementableEvents below
 * inside BP_ThirdPersonGameMode to add any Blueprint-side reactions.
 */
UCLASS()
class LABRYNTH_PROJECT_API ALabrynthGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALabrynthGameMode();

	// ── Configurable in Blueprint Class Defaults ──────────────────────────

	/** Pawn class spawned for survivor players (set to BP_ThirdPersonCharacter). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Labrynth|Classes")
	TSubclassOf<APawn> SurvivorClass;

	/** Pawn class spawned for the monster player (set to BP_NPC). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Labrynth|Classes")
	TSubclassOf<APawn> MonsterClass;

	/** Tag on Player Start actors that mark survivor spawn locations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Labrynth|Spawning")
	FName SurvivorSpawnTag = FName("SurvivorSpawn");

	/** Tag on the Player Start actor that marks the monster spawn location. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Labrynth|Spawning")
	FName MonsterSpawnTag = FName("MonsterSpawn");

	// ── Blueprint-callable helpers ────────────────────────────────────────

	/**
	 * Get the role of a given player controller.
	 * Call this from any Blueprint to check who is a survivor vs monster.
	 */
	UFUNCTION(BlueprintPure, Category = "Labrynth|Role")
	EPlayerRole GetRoleForController(APlayerController* Controller) const;

	/**
	 * End the game and declare a winner.
	 * Call this from Blueprint when the win/lose condition is met.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Labrynth|Game")
	void EndGame(bool bSurvivorsWin);

	// ── Blueprint-implementable events (override these in BP_ThirdPersonGameMode) ──

	/** Fired on the server when a survivor successfully joins and their pawn is spawned. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Labrynth|Events")
	void OnSurvivorJoined(APlayerController* SurvivorController, int32 SurvivorIndex);

	/** Fired on the server when the monster player joins and their pawn is spawned. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Labrynth|Events")
	void OnMonsterJoined(APlayerController* MonsterController);

	/** Fired when all 3 players have connected and the game is ready to begin. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Labrynth|Events")
	void OnAllPlayersReady();

protected:
	/** How many players have connected (incremented each PostLogin). */
	UPROPERTY(BlueprintReadOnly, Category = "Labrynth|State")
	int32 ConnectedPlayerCount = 0;

	// ── Overrides ─────────────────────────────────────────────────────────

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

private:
	/** Find a tagged Player Start actor by tag. Returns nullptr if not found. */
	AActor* FindSpawnPointByTag(FName Tag, int32 Index = 0) const;

	/** Spawn a pawn of PawnClass at SpawnPoint and have Controller possess it. */
	void SpawnAndPossess(APlayerController* Controller, TSubclassOf<APawn> PawnClass, AActor* SpawnPoint);
};
