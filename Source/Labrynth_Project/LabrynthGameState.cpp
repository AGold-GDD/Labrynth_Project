#include "LabrynthGameState.h"
#include "Net/UnrealNetwork.h"

void ALabrynthGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALabrynthGameState, ConnectedPlayerCount);
	DOREPLIFETIME(ALabrynthGameState, GamePhase);
}

void ALabrynthGameState::SetGamePhase(EGamePhase NewPhase)
{
	GamePhase = NewPhase;
	// Also call the event on the server (OnRep only fires on clients)
	OnGamePhaseChanged(NewPhase);
}

void ALabrynthGameState::OnRep_ConnectedCount()
{
	OnConnectedCountChanged(ConnectedPlayerCount);
}

void ALabrynthGameState::OnRep_GamePhase()
{
	OnGamePhaseChanged(GamePhase);
}
