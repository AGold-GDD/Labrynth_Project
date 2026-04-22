#include "Lab_PlayerController.h"
#include "Lab_PlayerState.h"
#include "Lab_GameInstance.h"

void ALab_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	FString Username;
	if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
		Username = GI->GetLocalUsername();

	Server_SetDisplayName(Username.IsEmpty() ? TEXT("Player") : Username);
}

void ALab_PlayerController::Server_SetDisplayName_Implementation(const FString& Name)
{
	if (ALab_PlayerState* PS = GetPlayerState<ALab_PlayerState>())
	{
		PS->SetDisplayName(Name);
	}
}

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
