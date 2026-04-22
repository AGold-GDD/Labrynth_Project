#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LabrynthPlayerState.generated.h"

/** Replicated role for each connected player. Readable from any Blueprint. */
UENUM(BlueprintType)
enum class EPlayerRole : uint8
{
	Unassigned  UMETA(DisplayName = "Unassigned"),
	Survivor    UMETA(DisplayName = "Survivor"),
	Monster     UMETA(DisplayName = "Monster")
};

UCLASS()
class LABRYNTH_PROJECT_API ALabrynthPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	/** This player's role — automatically synced to all clients. */
	UPROPERTY(ReplicatedUsing = OnRep_Role, BlueprintReadOnly, Category = "Labrynth|Role")
	EPlayerRole Role = EPlayerRole::Unassigned;

	/** Called on server to assign a role. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Labrynth|Role")
	void SetRole(EPlayerRole NewRole);

	/** Returns true if this player is the monster. Callable from any Blueprint. */
	UFUNCTION(BlueprintPure, Category = "Labrynth|Role")
	bool IsMonster() const { return Role == EPlayerRole::Monster; }

	/** Returns true if this player is a survivor. */
	UFUNCTION(BlueprintPure, Category = "Labrynth|Role")
	bool IsSurvivor() const { return Role == EPlayerRole::Survivor; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION()
	void OnRep_Role();
};
