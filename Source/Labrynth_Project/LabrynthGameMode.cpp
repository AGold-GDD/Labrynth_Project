#include "LabrynthGameMode.h"
#include "LabrynthPlayerState.h"
#include "LabrynthGameState.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ALabrynthGameMode::ALabrynthGameMode()
{
	// Use our custom classes so Blueprint children inherit them automatically
	PlayerStateClass  = ALabrynthPlayerState::StaticClass();
	GameStateClass    = ALabrynthGameState::StaticClass();

	// No automatic pawn spawning — we handle it manually in PostLogin
	DefaultPawnClass = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
//  PostLogin  — called on the SERVER each time a player connects
// ─────────────────────────────────────────────────────────────────────────────

void ALabrynthGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ConnectedPlayerCount++;

	// Update the replicated game state so all clients see the lobby count
	if (ALabrynthGameState* GS = GetGameState<ALabrynthGameState>())
	{
		GS->ConnectedPlayerCount = ConnectedPlayerCount;
	}

	// Assign role and spawn the correct pawn based on join order
	if (ConnectedPlayerCount == 1 || ConnectedPlayerCount == 2)
	{
		// ── Survivor ──────────────────────────────────────────────────────
		if (ALabrynthPlayerState* PS = NewPlayer->GetPlayerState<ALabrynthPlayerState>())
		{
			PS->SetRole(EPlayerRole::Survivor);
		}

		if (SurvivorClass)
		{
			// Index 0 for first survivor, 1 for second
			AActor* SpawnPoint = FindSpawnPointByTag(SurvivorSpawnTag, ConnectedPlayerCount - 1);
			SpawnAndPossess(NewPlayer, SurvivorClass, SpawnPoint);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LabrynthGameMode: SurvivorClass is not set! "
				"Set it in BP_ThirdPersonGameMode Class Defaults."));
		}

		OnSurvivorJoined(NewPlayer, ConnectedPlayerCount);
	}
	else if (ConnectedPlayerCount == 3)
	{
		// ── Monster ───────────────────────────────────────────────────────
		if (ALabrynthPlayerState* PS = NewPlayer->GetPlayerState<ALabrynthPlayerState>())
		{
			PS->SetRole(EPlayerRole::Monster);
		}

		if (MonsterClass)
		{
			AActor* SpawnPoint = FindSpawnPointByTag(MonsterSpawnTag, 0);
			SpawnAndPossess(NewPlayer, MonsterClass, SpawnPoint);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LabrynthGameMode: MonsterClass is not set! "
				"Set it in BP_ThirdPersonGameMode Class Defaults."));
		}

		OnMonsterJoined(NewPlayer);

		// All 3 players are in — start the game
		if (ALabrynthGameState* GS = GetGameState<ALabrynthGameState>())
		{
			GS->SetGamePhase(EGamePhase::InProgress);
		}

		OnAllPlayersReady();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  Logout  — called when a player disconnects
// ─────────────────────────────────────────────────────────────────────────────

void ALabrynthGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ConnectedPlayerCount = FMath::Max(0, ConnectedPlayerCount - 1);

	if (ALabrynthGameState* GS = GetGameState<ALabrynthGameState>())
	{
		GS->ConnectedPlayerCount = ConnectedPlayerCount;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  EndGame  — call from Blueprint when win/lose condition triggers
// ─────────────────────────────────────────────────────────────────────────────

void ALabrynthGameMode::EndGame(bool bSurvivorsWin)
{
	if (ALabrynthGameState* GS = GetGameState<ALabrynthGameState>())
	{
		GS->SetGamePhase(bSurvivorsWin ? EGamePhase::SurvivorsWin : EGamePhase::MonsterWins);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  GetRoleForController  — Blueprint Pure helper
// ─────────────────────────────────────────────────────────────────────────────

EPlayerRole ALabrynthGameMode::GetRoleForController(APlayerController* Controller) const
{
	if (!Controller) return EPlayerRole::None;

	if (const ALabrynthPlayerState* PS = Controller->GetPlayerState<ALabrynthPlayerState>())
	{
		return PS->PlayerRole;
	}

	return EPlayerRole::None;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Private helpers
// ─────────────────────────────────────────────────────────────────────────────

AActor* ALabrynthGameMode::FindSpawnPointByTag(FName Tag, int32 Index) const
{
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), Tag, Found);

	if (Found.IsValidIndex(Index))
	{
		return Found[Index];
	}

	// Fallback: return any PlayerStart if the tagged one wasn't found
	UE_LOG(LogTemp, Warning, TEXT("LabrynthGameMode: No spawn point with tag '%s' at index %d. "
		"Did you tag your Player Start actors in the level?"), *Tag.ToString(), Index);

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

void ALabrynthGameMode::SpawnAndPossess(APlayerController* Controller,
                                         TSubclassOf<APawn> PawnClass,
                                         AActor* SpawnPoint)
{
	if (!Controller || !PawnClass) return;

	FVector  Location = SpawnPoint ? SpawnPoint->GetActorLocation() : FVector::ZeroVector;
	FRotator Rotation = SpawnPoint ? SpawnPoint->GetActorRotation() : FRotator::ZeroRotator;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnClass, Location, Rotation, Params);

	if (NewPawn)
	{
		Controller->Possess(NewPawn);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("LabrynthGameMode: Failed to spawn pawn of class '%s'."),
			*PawnClass->GetName());
	}
}
