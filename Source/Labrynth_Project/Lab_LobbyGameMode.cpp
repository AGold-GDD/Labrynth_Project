#include "Lab_LobbyGameMode.h"
#include "Lab_PlayerController.h"

ALab_LobbyGameMode::ALab_LobbyGameMode()
{
	DefaultPawnClass = nullptr;
}

void ALab_LobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ALab_PlayerController* PC = Cast<ALab_PlayerController>(NewPlayer))
	{
		PC->EnableUIInputMode();
	}
}
