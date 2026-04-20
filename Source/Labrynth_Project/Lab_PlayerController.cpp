#include "Lab_PlayerController.h"
#include "Lab_PlayerState.h"

void ALab_PlayerController::EnableUIInputMode()
{
	FInputModeUIOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = true;
}

void ALab_PlayerController::EnableGameInputMode()
{
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;
}

bool ALab_PlayerController::IsMonsterPlayer() const
{
	const ALab_PlayerState* PS = GetPlayerState<ALab_PlayerState>();
	return PS && PS->PlayerRole == EPlayerRole::Monster;
}

bool ALab_PlayerController::IsLocalController_BP() const
{
	return IsLocalController();
}
