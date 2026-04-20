#include "Lab_PlayerState.h"
#include "Net/UnrealNetwork.h"

void ALab_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALab_PlayerState, PlayerRole);
	DOREPLIFETIME(ALab_PlayerState, bIsCaught);
}

void ALab_PlayerState::SetPlayerRole(EPlayerRole NewRole)
{
	PlayerRole = NewRole;
	// Manually call on the server — RepNotify only fires automatically on clients.
	OnRep_PlayerRole();
}

void ALab_PlayerState::SetCaught()
{
	if (bIsCaught) return; // Ignore repeat calls
	bIsCaught = true;
	OnRep_bIsCaught();
}

void ALab_PlayerState::OnRep_PlayerRole()
{
	OnRoleAssigned.Broadcast(PlayerRole);
}

void ALab_PlayerState::OnRep_bIsCaught()
{
	OnSurvivorCaught.Broadcast();
}
