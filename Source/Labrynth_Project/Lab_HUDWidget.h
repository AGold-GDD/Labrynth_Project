#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lab_Types.h"
#include "Lab_HUDWidget.generated.h"

class ALab_GameState;

/**
 * ULab_HUDWidget
 *
 * In-game HUD built entirely in Slate — no UMG Designer work needed.
 *
 * Create WBP_HUD as a Blueprint child in Content/MultiplayerStuff/
 * (right-click → User Interface → Widget Blueprint → parent = Lab_HUDWidget).
 * Leave the Designer completely empty. All layout comes from RebuildWidget().
 *
 * Set WBP_HUD as HUDWidgetClass in BP_Lab_HUD, then set BP_Lab_HUD as
 * HUDClass in BP_Lab_GameMode. That's the only Blueprint setup needed.
 *
 * HUD panels:
 *   InProgress    → top-center timer + round counter
 *   RoundEnding   → same timer (frozen) + "ROUND OVER" banner
 *   ShowingResults → full results leaderboard (sorted fastest first)
 */
UCLASS()
class LABRYNTH_PROJECT_API ULab_HUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	// ── Slate attribute callbacks (polled every frame by Slate) ───────────────

	FText GetTimerText() const;
	FText GetRoundText() const;

	// ── Delegate handlers ─────────────────────────────────────────────────────

	UFUNCTION()
	void HandlePhaseChanged(EGamePhase NewPhase);

	UFUNCTION()
	void HandleRoundResultsUpdated();

	// Rebuilds the dynamic leaderboard rows inside ResultsBox.
	void RebuildResultsList();

	// ── State ─────────────────────────────────────────────────────────────────

	TWeakObjectPtr<ALab_GameState> CachedGameState;
	EGamePhase CurrentPhase = EGamePhase::WaitingForPlayers;

	// Panel pointers — held so we can toggle visibility on phase change.
	TSharedPtr<SWidget> TimerPanel;
	TSharedPtr<SWidget> RoundEndingPanel;
	TSharedPtr<SWidget> ResultsPanel;

	// Inner box we clear and refill each time a new round result arrives.
	TSharedPtr<SVerticalBox> ResultsBox;
};
