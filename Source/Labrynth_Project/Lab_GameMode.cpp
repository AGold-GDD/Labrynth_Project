#include "Lab_GameMode.h"
#include "Lab_GameState.h"
#include "Lab_PlayerState.h"
#include "Lab_GameInstance.h"
#include "Lab_HUD.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"


int32 ALab_GameMode::GetSessionPlayerCount() const
{
	if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
		return FMath::Clamp(GI->SessionPlayerCount, 3, 6);
	return MaxPlayers;
}

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

	const int32 N = GetSessionPlayerCount();

	if (ConnectedPlayerCount < N)
	{
		SpawnAndPossess(NewPlayer, SurvivorPawnClass,
		                FindSpawnTransform(SurvivorSpawnTag, ConnectedPlayerCount - 1),
		                EPlayerRole::Survivor);
	}
	else if (ConnectedPlayerCount == N)
	{
		SpawnAndPossess(NewPlayer, MonsterPawnClass,
		                FindSpawnTransform(MonsterSpawnTag, 0), EPlayerRole::Monster);

		if (ALab_GameState* GS = GetGameState<ALab_GameState>())
		{
			GS->TotalRounds = N;
			GS->ResetForNewRound(1);
			GS->SetGamePhase(EGamePhase::WaitingToStart);
		}
	}
	else
	{
		UE_LOG(LogGameMode, Warning,
		       TEXT("ALab_GameMode: Player %d connected but session is for %d — no pawn spawned."),
		       ConnectedPlayerCount, N);
	}
}

void ALab_GameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--ConnectedPlayerCount;

	AllPlayers.Remove(Cast<APlayerController>(Exiting));

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
// Ready-up
// ─────────────────────────────────────────────────────────────────────────────

void ALab_GameMode::NotifyPlayerReady()
{
	++ReadyPlayerCount;

	if (ALab_GameState* GS = GetGameState<ALab_GameState>())
	{
		GS->SetReadyPlayerCount(ReadyPlayerCount);
	}

	if (ReadyPlayerCount >= AllPlayers.Num())
	{
		StartCurrentRound();
	}
}

void ALab_GameMode::StartCurrentRound()
{
	if (ALab_GameState* GS = GetGameState<ALab_GameState>())
	{
		GS->SetGamePhase(EGamePhase::InProgress);
		GS->StartRoundTimer();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Win / round evaluation
// ─────────────────────────────────────────────────────────────────────────────

void ALab_GameMode::EvaluateWinCondition()
{
	ALab_GameState* GS = GetGameState<ALab_GameState>();
	if (!GS || GS->GamePhase != EGamePhase::InProgress) return;

	if (CaughtSurvivorCount >= GetSessionPlayerCount() - 1)
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
	if (!GS || GS->GamePhase != EGamePhase::InProgress) return;

	GS->PauseRoundTimer();
	GS->SetGamePhase(EGamePhase::RoundEnding);

	// Record result for the monster who just played.
	FString MonsterName;
	float   ElapsedTime = GS->RoundElapsedTime;

	const int32 N = GetSessionPlayerCount();
	if (CurrentRoundIdx < N && CurrentRoundIdx < AllPlayers.Num())
	{
		int32 MonsterIdx = (N - 1 + CurrentRoundIdx) % N;
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

	if (CurrentRoundIdx >= GetSessionPlayerCount())
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
	ReadyPlayerCount = 0;
	CaughtSurvivorCount  = 0;
	EscapedSurvivorCount = 0;

	for (APlayerController* PC : AllPlayers)
	{
		if (!PC) continue;
		if (ALab_PlayerState* PS = PC->GetPlayerState<ALab_PlayerState>())
		{
			PS->ResetCaught();
			PS->ResetReady();
		}
	}

	const int32 N2 = GetSessionPlayerCount();
	RespawnAllPlayers((N2 - 1 + CurrentRoundIdx) % N2);

	if (ALab_GameState* GS = GetGameState<ALab_GameState>())
	{
		GS->ResetForNewRound(CurrentRoundIdx + 1);
		GS->SetGamePhase(EGamePhase::WaitingToStart);
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
	TArray<FTransform> Transforms;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (It->ActorHasTag(Tag))
			Transforms.Add(It->GetActorTransform());
	}

	if (Transforms.IsEmpty())
	{
		UE_LOG(LogGameMode, Warning,
		       TEXT("ALab_GameMode: No actors with tag '%s' found in the level."), *Tag.ToString());
		return FTransform::Identity;
	}

	return Transforms[Index % Transforms.Num()];
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
