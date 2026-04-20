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
	SurvivorsWin      UMETA(DisplayName = "Survivors Win"),
	MonsterWins       UMETA(DisplayName = "Monster Wins")
};
