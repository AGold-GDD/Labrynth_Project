#pragma once

#include "CoreMinimal.h"
#include "Lab_Types.generated.h"

// Roles assigned to each player when they join.
// The GameMode assigns these in PostLogin order: 1st = Survivor, 2nd = Survivor, 3rd = Monster.
UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
	None     UMETA(DisplayName = "None"),
	Survivor UMETA(DisplayName = "Survivor"),
	Monster  UMETA(DisplayName = "Monster")
};

// High-level game flow state. Replicated via GameState so all clients can read it.
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	WaitingForPlayers UMETA(DisplayName = "Waiting for Players"),
	InProgress        UMETA(DisplayName = "In Progress"),
	// Brief pause between rounds: timer is frozen, next round is about to begin.
	RoundEnding       UMETA(DisplayName = "Round Ending"),
	// All rounds complete; the leaderboard is shown.
	ShowingResults    UMETA(DisplayName = "Showing Results"),
	SurvivorsWin      UMETA(DisplayName = "Survivors Win"),
	MonsterWins       UMETA(DisplayName = "Monster Wins")
};

// One entry in the end-of-game leaderboard.
USTRUCT(BlueprintType)
struct FRoundResult
{
	GENERATED_BODY()

	// Username of the player who was monster that round.
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	FString PlayerName;

	// How many seconds it took them to catch all survivors.
	UPROPERTY(BlueprintReadOnly, Category = "Results")
	float TimeSeconds = 0.f;
};
