#include "LabrynthPlayerState.h"
#include "Net/UnrealNetwork.h"

void ALabrynthPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate Role to every connected client
	DOREPLIFETIME(ALabrynthPlayerState, PlayerRole);
}

void ALabrynthPlayerState::SetRole(EPlayerRole NewRole)
{
	// Authority-only — GameMode calls this from PostLogin
	PlayerRole = NewRole;
}

void ALabrynthPlayerState::OnRep_Role()
{
	// Fires on clients when Role changes — add UI update logic here
	// or override this event in a Blueprint child of ALabrynthPlayerState.
}
