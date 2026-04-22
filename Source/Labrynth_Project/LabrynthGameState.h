#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Lab_Types.h"
#include "LabrynthGameState.generated.h"

UCLASS()
class LABRYNTH_PROJECT_API ALabrynthGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	/** How many players have connected so far. Replicated. */
	UPROPERTY(ReplicatedUsing = OnRep_ConnectedCount, BlueprintReadOnly, Category = "Labrynth|Session")
	int32 ConnectedPlayerCount = 0;

	/** Current game phase. Replicated. */
	UPROPERTY(ReplicatedUsing = OnRep_GamePhase, BlueprintReadOnly, Category = "Labrynth|Session")
	EGamePhase GamePhase = EGamePhase::WaitingForPlayers;

	/** Called by GameMode on server to advance the phase. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Labrynth|Session")
	void SetGamePhase(EGamePhase NewPhase);

	/**
	 * Blueprint-implementable event fired on ALL clients when the game phase
	 * changes (e.g. show "Game Start!" banner, show win screens, etc.)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Labrynth|Session")
	void OnGamePhaseChanged(EGamePhase NewPhase);

	/** Fires on all clients when a new player count is replicated. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Labrynth|Session")
	void OnConnectedCountChanged(int32 NewCount);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_ConnectedCount();

	UFUNCTION()
	void OnRep_GamePhase();
};
