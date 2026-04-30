#include "Lab_GameMode.h"
#include "Lab_GameState.h"
#include "Lab_PlayerState.h"
#include "Lab_HUD.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

// Monster rotation order by AllPlayers index.
// Round 1: player[2] (3rd to join), Round 2: player[0], Round 3: player[1].
static const int32 MonsterOrder[3] = { 2, 0, 1 };
static constexpr int32 TotalRounds = 3;

ALab_GameMode::ALab_GameMode()
{
	DefaultPawnClass = nullptr;

	GameStateClass   = ALab_GameState::StaticClass();
	PlayerStateClass = ALab_PlayerState::StaticClass();
	HUDClass         = ALab_HUD::StaticClass();

	bUseSeamlessTravel = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Player connection
// ─────────────────────────────────────────────────────────────────────────────

void ALab_GameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	++ConnectedPlayerCount;
	AllPlayers.Add(NewPlayer);

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
			// All three players connected — start round 1
			if (ALab_GameState* GS = GetGameState<ALab_GameState>())
			{
				GS->ResetForNewRound(1);
				GS->SetGamePhase(EGamePhase::InProgress);
				GS->StartRoundTimer();
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

	if (ALab_GameState* GS = GetGameState<ALab_GameState>())
	{
		if (GS->GamePhase == EGamePhase::InProgress)
		{
			UE_LOG(LogGameMode, Log, TEXT("ALab_GameMode: Player disconnected during game."));
		}
		GS->SetConnectedPlayerCount(ConnectedPlayerCount);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Survivor caught
// ─────────────────────────────────────────────────────────────────────────────

void ALab_GameMode::NotifySurvivorCaught(APlayerController* CaughtPlayer)
{
	if (!CaughtPlayer) return;

	APawn* OldPawn = CaughtPlayer->GetPawn();
	UE_LOG(LogGameMode, Log, TEXT("NotifySurvivorCaught: controller=%s pawn=%s"),
		*CaughtPlayer->GetName(),
		OldPawn ? *OldPawn->GetName() : TEXT("NULL"));

	if (OldPawn)
	{
		CaughtPlayer->UnPossess();
		OldPawn->Destroy();
	}
	else
	{
		UE_LOG(LogGameMode, Warning, TEXT("NotifySurvivorCaught: GetPawn() was null — skipping respawn"));
		return;
	}

	if (ALab_PlayerState* PS = CaughtPlayer->GetPlayerState<ALab_PlayerState>())
	{
		PS->SetCaught();
	}

	++CaughtSurvivorCount;
	if (ALab_GameState* GS = GetGameState<ALab_GameState>())
	{
		GS->NotifySurvivorCaught();
	}

	// Respawn at a monster spawn point, not at the catch location.
	FTransform MonsterRespawnTransform = FindSpawnTransform(MonsterSpawnTag, 0);
	SpawnAndPossess(CaughtPlayer, MonsterPawnClass, MonsterRespawnTransform, EPlayerRole::Monster);

	EvaluateWinCondition();
}

void ALab_GameMode::NotifySurvivorEscaped()
{
	++EscapedSurvivorCount;
	EvaluateWinCondition();
}

// ─────────────────────────────────────────────────────────────────────────────
// Win / round evaluation
// ─────────────────────────────────────────────────────────────────────────────

void ALab_GameMode::EvaluateWinCondition()
{
	ALab_GameState* GS = GetGameState<ALab_GameState>();
	if (!GS || GS->GamePhase != EGamePhase::InProgress) return;

	if (CaughtSurvivorCount >= TotalSurvivors)
	{
		FinishCurrentRound();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Round management
// ─────────────────────────────────────────────────────────────────────────────

void ALab_GameMode::FinishCurrentRound()
{
	ALab_GameState* GS = GetGameState<ALab_GameState>();
	if (!GS) return;

	GS->PauseRoundTimer();
	GS->SetGamePhase(EGamePhase::RoundEnding);

	// Record result for the monster who just played.
	FString MonsterName;
	float   ElapsedTime = GS->RoundElapsedTime;

	if (CurrentRoundIdx < TotalRounds && CurrentRoundIdx < AllPlayers.Num())
	{
		int32 MonsterIdx = MonsterOrder[CurrentRoundIdx];
		if (MonsterIdx < AllPlayers.Num() && AllPlayers[MonsterIdx])
		{
			if (ALab_PlayerState* PS = AllPlayers[MonsterIdx]->GetPlayerState<ALab_PlayerState>())
			{
				MonsterName = PS->DisplayName.IsEmpty() ? PS->GetPlayerName() : PS->DisplayName;
			}
		}
	}

	GS->AddRoundResult(MonsterName, ElapsedTime);
	OnRoundComplete(CurrentRoundIdx + 1, MonsterName, ElapsedTime);

	GetWorldTimerManager().SetTimer(
		RoundTransitionHandle, this,
		&ALab_GameMode::OnRoundTransitionComplete,
		RoundTransitionDelay, false);
}

void ALab_GameMode::OnRoundTransitionComplete()
{
	++CurrentRoundIdx;

	if (CurrentRoundIdx >= TotalRounds)
	{
		// All players have had a turn as monster — show the leaderboard.
		OnAllRoundsComplete();
		if (ALab_GameState* GS = GetGameState<ALab_GameState>())
		{
			GS->SetGamePhase(EGamePhase::ShowingResults);
		}
		return;
	}

	// Reset per-round state on all PlayerStates.
	for (APlayerController* PC : AllPlayers)
	{
		if (!PC) continue;
		if (ALab_PlayerState* PS = PC->GetPlayerState<ALab_PlayerState>())
		{
			PS->ResetCaught();
		}
	}

	CaughtSurvivorCount  = 0;
	EscapedSurvivorCount = 0;

	RespawnAllPlayers(MonsterOrder[CurrentRoundIdx]);

	if (ALab_GameState* GS = GetGameState<ALab_GameState>())
	{
		GS->ResetForNewRound(CurrentRoundIdx + 1);
		GS->SetGamePhase(EGamePhase::InProgress);
		GS->StartRoundTimer();
	}
}

void ALab_GameMode::RespawnAllPlayers(int32 MonsterPlayerIdx)
{
	int32 SurvivorSpawnIdx = 0;

	for (int32 i = 0; i < AllPlayers.Num(); ++i)
	{
		APlayerController* PC = AllPlayers[i];
		if (!PC) continue;

		// Destroy existing pawn before respawning.
		if (APawn* OldPawn = PC->GetPawn())
		{
			PC->UnPossess();
			OldPawn->Destroy();
		}

		if (i == MonsterPlayerIdx)
		{
			SpawnAndPossess(PC, MonsterPawnClass,
			                FindSpawnTransform(MonsterSpawnTag, 0), EPlayerRole::Monster);
		}
		else
		{
			SpawnAndPossess(PC, SurvivorPawnClass,
			                FindSpawnTransform(SurvivorSpawnTag, SurvivorSpawnIdx), EPlayerRole::Survivor);
			++SurvivorSpawnIdx;
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Blueprint native events
// ─────────────────────────────────────────────────────────────────────────────

void ALab_GameMode::OnRoundComplete_Implementation(int32 RoundNumber, const FString& MonsterName, float TimeSeconds)
{
	// Default: nothing. Override in BP_Lab_GameMode to play a sound or animation.
}

void ALab_GameMode::OnAllRoundsComplete_Implementation()
{
	// Default: nothing. Override in BP_Lab_GameMode for any final cleanup.
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

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
	       TEXT("ALab_GameMode: No actor with tag '%s' at index %d found in the level."),
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
	Params.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	APawn* Pawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, Params);
	if (!Pawn)
	{
		UE_LOG(LogGameMode, Error, TEXT("ALab_GameMode::SpawnAndPossess: SpawnActor failed."));
		return;
	}

	PC->Possess(Pawn);

	if (ALab_PlayerState* PS = PC->GetPlayerState<ALab_PlayerState>())
	{
		PS->SetPlayerRole(AssignedRole);
	}
}
