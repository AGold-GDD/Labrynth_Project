#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Lab_MonsterCharacter.generated.h"

class UCameraComponent;
class USphereComponent;
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
 *   - Mesh as usual (it is hidden from the monster's own screen automatically)
 *
 * TagSphere radius can be adjusted in the Blueprint's component details.
 */
UCLASS()
class LABRYNTH_PROJECT_API ALab_MonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALab_MonsterCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// Eye-level first-person camera — no spring arm
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCamera;

	// Catching range — overlapping a survivor calls NotifySurvivorCaught on the server
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tagging")
	USphereComponent* TagSphere;

	// Assign these in BP_MonsterCharacter Class Defaults → Input section
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	UFUNCTION()
	void OnTagSphereOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                        bool bFromSweep, const FHitResult& SweepResult);
};
