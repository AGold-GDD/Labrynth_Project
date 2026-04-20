#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Lab_LobbyGameMode.generated.h"

/**
 * ALab_LobbyGameMode
 *
 * Used only in the Lobby map. Does nothing except prevent pawn spawning.
 * Set this as the GameMode Override in the Lobby map's World Settings.
 */
UCLASS()
class LABRYNTH_PROJECT_API ALab_LobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALab_LobbyGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
};
