#include "Lab_GameState.h"
#include "Net/UnrealNetwork.h"

ALab_GameState::ALab_GameState()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	// Higher update frequency so the replicated timer appears smooth on clients.
	SetNetUpdateFrequency(20.f);
}

void ALab_GameState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Only the server increments the timer; replication pushes the value to clients.
	if (HasAuthority() && bTimerActive)
	{
		RoundElapsedTime += DeltaTime;
	}
}

void ALab_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALab_GameState, GamePhase);
	DOREPLIFETIME(ALab_GameState, ConnectedPlayerCount);
	DOREPLIFETIME(ALab_GameState, CaughtSurvivorCount);
	DOREPLIFETIME(ALab_GameState, RoundElapsedTime);
	DOREPLIFETIME(ALab_GameState, CurrentRound);
	DOREPLIFETIME(ALab_GameState, RoundResults);
	DOREPLIFETIME(ALab_GameState, ReadyPlayerCount);
}

void ALab_GameState::SetGamePhase(EGamePhase NewPhase)
{
	GamePhase = NewPhase;
	OnRep_GamePhase(); // Manually fire on the server; RepNotify auto-fires on clients
}

void ALab_GameState::SetConnectedPlayerCount(int32 Count)
{
	ConnectedPlayerCount = Count;
	OnRep_ConnectedPlayerCount();
}

void ALab_GameState::NotifySurvivorCaught()
{
	++CaughtSurvivorCount;
	OnRep_CaughtSurvivorCount();
}

void ALab_GameState::StartRoundTimer()
{
	bTimerActive = true;
}

void ALab_GameState::PauseRoundTimer()
{
	bTimerActive = false;
}

void ALab_GameState::AddRoundResult(const FString& PlayerName, float TimeSeconds)
{
	FRoundResult Result;
	Result.PlayerName = PlayerName;
	Result.TimeSeconds = TimeSeconds;
	RoundResults.Add(Result);
	OnRep_RoundResults(); // Manually fire on the server
}

void ALab_GameState::SetReadyPlayerCount(int32 Count)
{
	ReadyPlayerCount = Count;
	OnRep_ReadyPlayerCount();
}

void ALab_GameState::ResetForNewRound(int32 NewRoundNumber)
{
	RoundElapsedTime = 0.f;
	CaughtSurvivorCount = 0;
	ReadyPlayerCount = 0;
	CurrentRound = NewRoundNumber;
	bTimerActive = false;
}

void ALab_GameState::OnRep_GamePhase()
{
	OnGamePhaseChanged.Broadcast(GamePhase);
}

void ALab_GameState::OnRep_ConnectedPlayerCount()
{
	OnConnectedCountChanged.Broadcast(ConnectedPlayerCount);
}

void ALab_GameState::OnRep_CaughtSurvivorCount()
{
	// GameMode handles win condition. This RepNotify is a hook for Blueprint
	// if you want to play an animation or sound on all clients when a survivor is caught.
}

void ALab_GameState::OnRep_ReadyPlayerCount()
{
	OnReadyCountChanged.Broadcast(ReadyPlayerCount);
}

void ALab_GameState::OnRep_RoundResults()
{
	OnRoundResultsUpdated.Broadcast();
}
