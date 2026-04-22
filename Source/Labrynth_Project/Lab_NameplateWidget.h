#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lab_NameplateWidget.generated.h"

class ACharacter;

/**
 * ULab_NameplateWidget
 *
 * Displays the player's DisplayName above their character.
 * Attached via a UWidgetComponent in Screen space — always faces the camera.
 *
 * No Blueprint setup needed. Characters create and configure this in C++.
 * SetOwningCharacter is called automatically by the character in BeginPlay.
 */
UCLASS()
class LABRYNTH_PROJECT_API ULab_NameplateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetOwningCharacter(ACharacter* Char);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TWeakObjectPtr<ACharacter> OwningChar;

	FText GetNameText() const;
};
