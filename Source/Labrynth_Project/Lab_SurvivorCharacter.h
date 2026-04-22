#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Lab_SurvivorCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * ALab_SurvivorCharacter
 *
 * Third-person survivor pawn. Set this as SurvivorPawnClass in BP_Lab_GameMode.
 *
 * Create a Blueprint child (BP_SurvivorCharacter) and assign:
 *   - DefaultMappingContext  →  IMC_Default  (ThirdPerson/Input/)
 *   - MoveAction             →  IA_Move      (ThirdPerson/Input/)
 *   - LookAction             →  IA_Look      (ThirdPerson/Input/)
 *   - JumpAction             →  IA_Jump      (ThirdPerson/Input/)
 *   - Mesh / Anim Blueprint as usual
 */
UCLASS()
class LABRYNTH_PROJECT_API ALab_SurvivorCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALab_SurvivorCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UWidgetComponent* NameplateWidget;

	// Assign these in BP_SurvivorCharacter Class Defaults → Input section
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputMappingContext* MouseLookMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UInputAction* JumpAction;

private:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

	// Fires only on the locally controlled machine when this survivor is caught.
	// Add a "CAUGHT" widget or camera effect here.
	UFUNCTION()
	void OnCaught();
};
