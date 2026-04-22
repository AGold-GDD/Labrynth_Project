#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Lab_Types.h"
#include "LabrynthPlayerState.generated.h"

UCLASS()
class LABRYNTH_PROJECT_API ALabrynthPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/** This player's role — automatically synced to all clients. */
	UPROPERTY(ReplicatedUsing = OnRep_Role, BlueprintReadOnly, Category = "Labrynth|Role")
	EPlayerRole PlayerRole = EPlayerRole::None;

	/** Called on server to assign a role. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Labrynth|Role")
	void SetRole(EPlayerRole NewRole);

	/** Returns true if this player is the monster. Callable from any Blueprint. */
	UFUNCTION(BlueprintPure, Category = "Labrynth|Role")
	bool IsMonster() const { return PlayerRole == EPlayerRole::Monster; }

	/** Returns true if this player is a survivor. */
	UFUNCTION(BlueprintPure, Category = "Labrynth|Role")
	bool IsSurvivor() const { return PlayerRole == EPlayerRole::Survivor; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_Role();
};
