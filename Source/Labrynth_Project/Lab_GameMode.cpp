#include "Lab_GameMode.h"
#include "Lab_GameState.h"
#include "Lab_PlayerState.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

ALab_GameMode::ALab_GameMode()
{
	// Disable UE's automatic pawn spawning.
	// We handle it manually in PostLogin so we can assign the right class per player.
	DefaultPawnClass = nullptr;

	// Use our custom replicated classes
	GameStateClass  = ALab_GameState::StaticClass();
	PlayerStateClass = ALab_PlayerState::StaticClass();

	// Seamless travel keeps all players connected when the server changes maps.
	// Requires a Transition Map set in Project Settings → Engine → General Settings.
	bUseSeamlessTravel = true;
}

void ALab_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	++ConnectedPlayerCount;

	// Update the replicated player count so lobby UIs can display "X/3 players connected"
	if (ALab_GameState* GS = GetGameState<ALab_GameState>())
	{
		GS->SetConnectedPlayerCount(ConnectedPlayerCount);
	}

	switch (ConnectedPlayerCount)
	{
		case 1:
			SpawnAndPossess(NewPlayer, SurvivorPawnClass,
			                FindSpawnTransform(SurvivorSpawnTag, 0), EPlayerRole::Survivor);
			break;
		case 2:
			SpawnAndPossess(NewPlayer, SurvivorPawnClass,
			                FindSpawnTransform(SurvivorSpawnTag, 1), EPlayerRole::Survivor);
			break;
		case 3:
			SpawnAndPossess(NewPlayer, MonsterPawnClass,
			                FindSpawnTransform(MonsterSpawnTag, 0), EPlayerRole::Monster);
			// All three players are connected — start the game
			if (ALab_GameState* GS = GetGameState<ALab_GameState>())
			{
				GS->SetGamePhase(EGamePhase::InProgress);
			}
			break;
		default:
			UE_LOG(LogGameMode, Warning,
			       TEXT("ALab_GameMode: More than 3 players connected (player %d). No pawn spawned."),
			       ConnectedPlayerCount);
			break;
	}
}

void ALab_GameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--ConnectedPlayerCount;

	// If a player disconnects mid-game, end the session gracefully.
	// You can change this behaviour (e.g. allow bots to fill in) in BP_Lab_GameMode.
	if (ALab_GameState* GS = GetGameState<ALab_GameState>())
	{
		if (GS->GamePhase == EGamePhase::InProgress)
		{
			UE_LOG(LogGameMode, Log, TEXT("ALab_GameMode: Player disconnected during game."));
			// Optionally call OnMonsterWins or OnSurvivorsWin depending on who left.
		}
		GS->SetConnectedPlayerCount(ConnectedPlayerCount);
	}
}

void ALab_GameMode::NotifySurvivorCaught(APlayerController* CaughtPlayer)
{
	if (!CaughtPlayer) return;

	APawn* OldPawn = CaughtPlayer->GetPawn();
	UE_LOG(LogGameMode, Log, TEXT("NotifySurvivorCaught: controller=%s pawn=%s"),
		*CaughtPlayer->GetName(),
		OldPawn ? *OldPawn->GetName() : TEXT("NULL"));

	// Save the survivor's position before destroying their pawn
	FTransform SpawnTransform = FTransform::Identity;
	if (OldPawn)
	{
		SpawnTransform = OldPawn->GetActorTransform();
		CaughtPlayer->UnPossess();
		OldPawn->Destroy();
		UE_LOG(LogGameMode, Log, TEXT("NotifySurvivorCaught: old pawn destroyed, spawning monster"));
	}
	else
	{
		UE_LOG(LogGameMode, Warning, TEXT("NotifySurvivorCaught: GetPawn() was null — skipping respawn"));
		return;
	}

	// Mark as caught in PlayerState
	if (ALab_PlayerState* PS = CaughtPlayer->GetPlayerState<ALab_PlayerState>())
	{
		PS->SetCaught();
	}

	++CaughtSurvivorCount;
	if (ALab_GameState* GS = GetGameState<ALab_GameState>())
	{
		GS->NotifySurvivorCaught();
	}

	// Respawn this player as a monster at the spot they were caught
	SpawnAndPossess(CaughtPlayer, MonsterPawnClass, SpawnTransform, EPlayerRole::Monster);

	EvaluateWinCondition();
}

void ALab_GameMode::NotifySurvivorEscaped()
{
	++EscapedSurvivorCount;
	EvaluateWinCondition();
}

void ALab_GameMode::EvaluateWinCondition()
{
	ALab_GameState* GS = GetGameState<ALab_GameState>();
	if (!GS || GS->GamePhase != EGamePhase::InProgress) return;

	int32 RemainingFree = TotalSurvivors - CaughtSurvivorCount - EscapedSurvivorCount;

	if (CaughtSurvivorCount >= TotalSurvivors)
	{
		// All survivors caught — monster wins
		GS->SetGamePhase(EGamePhase::MonsterWins);
		OnMonsterWins();
	}
	else if (EscapedSurvivorCount > 0 && RemainingFree == 0)
	{
		// All free survivors escaped — survivors win
		GS->SetGamePhase(EGamePhase::SurvivorsWin);
		OnSurvivorsWin();
	}
}

void ALab_GameMode::OnMonsterWins_Implementation()
{
	// Default: nothing. Override in BP_Lab_GameMode to show UI, play cinematics, etc.
}

void ALab_GameMode::OnSurvivorsWin_Implementation()
{
	// Default: nothing. Override in BP_Lab_GameMode.
}

FTransform ALab_GameMode::FindSpawnTransform(FName Tag, int32 Index) const
{
	int32 Found = 0;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(Tag))
		{
			if (Found == Index)
			{
				return It->GetActorTransform();
			}
			++Found;
		}
	}

	UE_LOG(LogGameMode, Warning,
	       TEXT("ALab_GameMode: No actor with tag '%s' at index %d found in the level. "
	            "Check that Player Start actors have the correct tags."),
	       *Tag.ToString(), Index);

	return FTransform::Identity;
}

void ALab_GameMode::SpawnAndPossess(APlayerController* PC, TSubclassOf<APawn> PawnClass,
                                    const FTransform& SpawnTransform, EPlayerRole AssignedRole)
{
	if (!PC)
	{
		UE_LOG(LogGameMode, Error, TEXT("ALab_GameMode::SpawnAndPossess: PlayerController is null"));
		return;
	}

	if (!PawnClass)
	{
		UE_LOG(LogGameMode, Error,
		       TEXT("ALab_GameMode::SpawnAndPossess: PawnClass is null for role %d. "
		            "Set SurvivorPawnClass / MonsterPawnClass in BP_Lab_GameMode's Class Defaults."),
		       static_cast<int32>(AssignedRole));
		return;
	}

	FActorSpawnParameters Params;
	// AdjustIfPossibleButAlwaysSpawn prevents spawn failures when two players
	// connect nearly simultaneously and their pawns overlap at the spawn point.
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* Pawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, Params);
	if (!Pawn)
	{
		UE_LOG(LogGameMode, Error, TEXT("ALab_GameMode::SpawnAndPossess: SpawnActor failed."));
		return;
	}

	PC->Possess(Pawn);

	// Store the role in PlayerState so all machines can read it
	if (ALab_PlayerState* PS = PC->GetPlayerState<ALab_PlayerState>())
	{
		PS->SetPlayerRole(AssignedRole);
	}
}
