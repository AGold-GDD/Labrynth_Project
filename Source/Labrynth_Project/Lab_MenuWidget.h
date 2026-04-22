#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lab_MenuWidget.generated.h"

class SEditableTextBox;

/**
 * ULab_MenuWidget
 *
 * Host / Join menu built entirely in Slate (no UMG Designer needed).
 *
 * Create a Blueprint child (WBP_Menu) in Content/MultiplayerStuff/:
 *   Right-click → User Interface → Widget Blueprint → pick Lab_MenuWidget as parent.
 * Then use WBP_Menu in the Lobby Level Blueprint → Create Widget as before.
 * No graph wiring or Designer work needed in the Blueprint child.
 */
UCLASS()
class LABRYNTH_PROJECT_API ULab_MenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	TSharedPtr<SEditableTextBox> UsernameInputBox;
	TSharedPtr<SEditableTextBox> IPInputBox;

	TArray<TSharedPtr<FString>> MapOptions;
	TSharedPtr<FString> SelectedMap;

	TArray<TSharedPtr<FString>> HostOptions;
	TSharedPtr<FString> SelectedHost;

	FReply OnHostClicked();
	FReply OnJoinClicked();

	void SaveUsername() const;
	TSharedRef<SWidget> MakeMapOptionWidget(TSharedPtr<FString> Item) const;
	FText GetSelectedMapText() const;
	TSharedRef<SWidget> MakeHostOptionWidget(TSharedPtr<FString> Item) const;
	FText GetSelectedHostText() const;
};
