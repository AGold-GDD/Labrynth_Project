#include "Lab_GameState.h"
#include "Net/UnrealNetwork.h"

ALab_GameState::ALab_GameState() {}

void ALab_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALab_GameState, GamePhase);
	DOREPLIFETIME(ALab_GameState, ConnectedPlayerCount);
	DOREPLIFETIME(ALab_GameState, CaughtSurvivorCount);
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
