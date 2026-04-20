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
 * Responsibilities:
 *   - PostLogin:  counts connecting players, spawns the correct pawn, possesses it,
 *                 assigns the player's role in their PlayerState.
 *   - Logout:     decrements the count and ends the game if not enough players remain.
 *   - NotifySurvivorCaught: called by the monster character when it tags a survivor.
 *                 Checks if all survivors are caught (monster wins) or not.
 *   - CheckSurvivorEscape:  called when a survivor reaches the exit.
 *                 Checks if the remaining free survivors have escaped (survivors win).
 *
 * Create a Blueprint child (BP_Lab_GameMode) in Content/MultiplayerStuff/.
 * Set SurvivorPawnClass and MonsterPawnClass there in the Class Defaults.
 * Then set BP_Lab_GameMode as the GameMode in DefaultEngine.ini.
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

	// Call this from the exit trigger in the maze level when a survivor overlaps it.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Game|WinCondition")
	void NotifySurvivorEscaped();

protected:
	// ── Set these in BP_Lab_GameMode's Class Defaults ─────────────────────────

	// Pawn spawned for each survivor player (player 1 and player 2).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	TSubclassOf<APawn> SurvivorPawnClass;

	// Pawn spawned for the monster player (player 3).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	TSubclassOf<APawn> MonsterPawnClass;

	// Actor tag used to identify survivor spawn points placed in the maze level.
	// Place two Player Start actors in the level and give them this tag.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	FName SurvivorSpawnTag = TEXT("SurvivorSpawn");

	// Actor tag used to identify the single monster spawn point.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	FName MonsterSpawnTag = TEXT("MonsterSpawn");

	// Total number of survivors in the game. Drives win condition checks.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game|Setup")
	int32 TotalSurvivors = 2;

	// ── Optional Blueprint overrides ──────────────────────────────────────────

	// Called on the server when the game transitions to MonsterWins.
	// Override in BP_Lab_GameMode to show a game-over UI or play a cutscene.
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "Game|WinCondition")
	void OnMonsterWins();
	virtual void OnMonsterWins_Implementation();

	// Called on the server when the game transitions to SurvivorsWin.
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "Game|WinCondition")
	void OnSurvivorsWin();
	virtual void OnSurvivorsWin_Implementation();

private:
	int32 ConnectedPlayerCount = 0;
	int32 CaughtSurvivorCount = 0;
	int32 EscapedSurvivorCount = 0;

	// Returns the world transform of the Nth actor with the given tag.
	// Returns FTransform::Identity and logs a warning if none is found.
	FTransform FindSpawnTransform(FName Tag, int32 Index) const;

	// Spawns a pawn of PawnClass at SpawnTransform, possesses it with PC,
	// and sets the player's role in their PlayerState.
	void SpawnAndPossess(APlayerController* PC, TSubclassOf<APawn> PawnClass,
	                     const FTransform& SpawnTransform, EPlayerRole AssignedRole);

	// Evaluates the win condition after a survivor is caught or escapes.
	void EvaluateWinCondition();
};
