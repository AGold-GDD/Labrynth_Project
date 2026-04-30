#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Lab_MonsterCharacter.generated.h"

class UCameraComponent;
class UWidgetComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * ALab_MonsterCharacter
 *
 * First-person monster pawn. Set this as MonsterPawnClass in BP_Lab_GameMode.
 *
 * Create a Blueprint child (BP_MonsterCharacter) and assign:
 *   - DefaultMappingContext  →  IMC_Default  (ThirdPerson/Input/)
 *   - MoveAction             →  IA_Move      (ThirdPerson/Input/)
 *   - LookAction             →  IA_Look      (ThirdPerson/Input/)
 *   - TagAction              →  IA_Tag       (create this — Digital bool, map to LMB + E)
 *   - Mesh as usual (it is hidden from the monster's own screen automatically)
 */
UCLASS()
class LABRYNTH_PROJECT_API ALab_MonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALab_MonsterCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Eye-level first-person camera — no spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* NameplateWidget;

	// Assign these in BP_MonsterCharacter Class Defaults → Input section
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	// Create IA_Tag (Digital bool) and map it to Left Mouse Button + E in IMC_Default.
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* TagAction;

	// How far the tag raycast reaches (in cm). 300 = roughly arm's reach in a corridor.
	UPROPERTY(EditDefaultsOnly, Category = "Tagging")
	float TagRange = 300.f;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// Fires on the locally controlled client: draws the debug ray, then calls server.
	void PerformTag();

	// Server runs the authoritative trace and calls NotifySurvivorCaught if it hits.
	UFUNCTION(Server, Reliable)
	void Server_TryTag();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSurvivorTagged(const FString& SurvivorName);
};
