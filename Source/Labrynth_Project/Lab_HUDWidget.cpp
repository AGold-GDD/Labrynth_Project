#include "Lab_HUDWidget.h"
#include "Lab_GameState.h"
#include "Lab_PlayerState.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Styling/CoreStyle.h"

// ─── Layout constants ─────────────────────────────────────────────────────────

static const FLinearColor PanelBg    = FLinearColor(0.f, 0.f, 0.f, 0.55f);
static const FLinearColor White      = FLinearColor::White;
static const FMargin      TimerPad   = FMargin(20.f, 10.f);
static const FMargin      BannerPad  = FMargin(40.f, 20.f);
static const FMargin      ResultsPad = FMargin(50.f, 30.f);

// ─── RebuildWidget ────────────────────────────────────────────────────────────

TSharedRef<SWidget> ULab_HUDWidget::RebuildWidget()
{
	// ------------------------------------------------------------------
	//  Ready-up panel — shown during WaitingToStart
	// ------------------------------------------------------------------
	TSharedRef<SBorder> ReadyWidget =
		SNew(SBorder)
		.BorderBackgroundColor(PanelBg)
		.Padding(BannerPad)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(STextBlock)
			.Text_UObject(this, &ULab_HUDWidget::GetReadyText)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
			.ColorAndOpacity(White)
			.Justification(ETextJustify::Center)
		];
	ReadyPanel = ReadyWidget;

	// ------------------------------------------------------------------
	//  Timer panel — shown during InProgress and RoundEnding
	// ------------------------------------------------------------------
	TSharedRef<SBorder> TimerWidget =
		SNew(SBorder)
		.BorderBackgroundColor(PanelBg)
		.Padding(TimerPad)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(SVerticalBox)

			// "Round 1 / 3"
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text_UObject(this, &ULab_HUDWidget::GetRoundText)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				.ColorAndOpacity(White)
			]

			// "00:00.00"
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text_UObject(this, &ULab_HUDWidget::GetTimerText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
				.ColorAndOpacity(White)
			]
		];
	TimerPanel = TimerWidget;

	// ------------------------------------------------------------------
	//  Round-ending banner — shown only during RoundEnding
	// ------------------------------------------------------------------
	TSharedRef<SBorder> RoundEndingWidget =
		SNew(SBorder)
		.BorderBackgroundColor(PanelBg)
		.Padding(BannerPad)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("ROUND OVER")))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
			.ColorAndOpacity(White)
		];
	RoundEndingPanel = RoundEndingWidget;

	// ------------------------------------------------------------------
	//  Results panel — shown during ShowingResults
	// ------------------------------------------------------------------
	TSharedPtr<SVerticalBox> ResultsContainer;
	TSharedRef<SBorder> ResultsWidget =
		SNew(SBorder)
		.BorderBackgroundColor(PanelBg)
		.Padding(ResultsPad)
		.Visibility(EVisibility::Collapsed)
		[
			SNew(SVerticalBox)

			// Title
			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 24.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("RESULTS")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 36))
				.ColorAndOpacity(White)
			]

			// Dynamic rows — populated by RebuildResultsList()
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SAssignNew(ResultsContainer, SVerticalBox)
			]
		];
	ResultsBox   = ResultsContainer;
	ResultsPanel = ResultsWidget;

	// ------------------------------------------------------------------
	//  Root overlay — all panels stack on top of the game view
	// ------------------------------------------------------------------
	return SNew(SOverlay)

		// Ready-up prompt — screen center
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			ReadyWidget
		]

		// Timer — top center
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(0.f, 24.f, 0.f, 0.f)
		[
			TimerWidget
		]

		// Round-ending banner — screen center
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			RoundEndingWidget
		]

		// Results leaderboard — screen center
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			ResultsWidget
		];
}

// ─── NativeConstruct — bind to GameState delegates ───────────────────────────

void ULab_HUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWorld* World = GetWorld())
	{
		CachedGameState = World->GetGameState<ALab_GameState>();
	}

	if (ALab_GameState* GS = CachedGameState.Get())
	{
		GS->OnGamePhaseChanged.AddDynamic(this, &ULab_HUDWidget::HandlePhaseChanged);
		GS->OnRoundResultsUpdated.AddDynamic(this, &ULab_HUDWidget::HandleRoundResultsUpdated);

		// Sync immediately with whatever phase is already active.
		HandlePhaseChanged(GS->GamePhase);
	}
}

// ─── Slate attribute callbacks ────────────────────────────────────────────────

FText ULab_HUDWidget::GetTimerText() const
{
	float T = 0.f;
	if (ALab_GameState* GS = CachedGameState.Get())
	{
		T = GS->RoundElapsedTime;
	}

	const int32 Minutes      = FMath::FloorToInt(T / 60.f);
	const int32 Seconds      = FMath::FloorToInt(FMath::Fmod(T, 60.f));
	const int32 Centiseconds = FMath::FloorToInt(FMath::Fmod(T, 1.f) * 100.f);

	return FText::FromString(
		FString::Printf(TEXT("%02d:%02d.%02d"), Minutes, Seconds, Centiseconds));
}

FText ULab_HUDWidget::GetRoundText() const
{
	int32 Current = 1;
	if (ALab_GameState* GS = CachedGameState.Get())
	{
		Current = GS->CurrentRound;
	}
	return FText::FromString(FString::Printf(TEXT("Round %d / 3"), Current));
}

FText ULab_HUDWidget::GetReadyText() const
{
	ALab_GameState* GS = CachedGameState.Get();
	const int32 ReadyCount  = GS ? GS->ReadyPlayerCount : 0;
	const int32 TotalPlayers = 3;

	// Check whether the local player has already readied up
	bool bLocalReady = false;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ALab_PlayerState* PS = PC->GetPlayerState<ALab_PlayerState>())
		{
			bLocalReady = PS->bIsReady;
		}
	}

	if (bLocalReady)
	{
		return FText::FromString(
			FString::Printf(TEXT("Ready!\n%d / %d players ready"), ReadyCount, TotalPlayers));
	}
	return FText::FromString(
		FString::Printf(TEXT("Press Enter to Ready Up\n%d / %d players ready"), ReadyCount, TotalPlayers));
}

// ─── Delegate handlers ────────────────────────────────────────────────────────

void ULab_HUDWidget::HandlePhaseChanged(EGamePhase NewPhase)
{
	CurrentPhase = NewPhase;

	const bool bShowReady    = (NewPhase == EGamePhase::WaitingToStart);
	const bool bShowTimer    = (NewPhase == EGamePhase::InProgress || NewPhase == EGamePhase::RoundEnding);
	const bool bShowRoundEnd = (NewPhase == EGamePhase::RoundEnding);
	const bool bShowResults  = (NewPhase == EGamePhase::ShowingResults);

	if (ReadyPanel.IsValid())
		ReadyPanel->SetVisibility(bShowReady ? EVisibility::Visible : EVisibility::Collapsed);

	if (TimerPanel.IsValid())
		TimerPanel->SetVisibility(bShowTimer ? EVisibility::Visible : EVisibility::Collapsed);

	if (RoundEndingPanel.IsValid())
		RoundEndingPanel->SetVisibility(bShowRoundEnd ? EVisibility::Visible : EVisibility::Collapsed);

	if (ResultsPanel.IsValid())
		ResultsPanel->SetVisibility(bShowResults ? EVisibility::Visible : EVisibility::Collapsed);
}

void ULab_HUDWidget::HandleRoundResultsUpdated()
{
	RebuildResultsList();
}

void ULab_HUDWidget::RebuildResultsList()
{
	if (!ResultsBox.IsValid()) return;

	ALab_GameState* GS = CachedGameState.Get();
	if (!GS) return;

	// Sort a copy — fastest time first (winner at top).
	TArray<FRoundResult> Sorted = GS->RoundResults;
	Sorted.Sort([](const FRoundResult& A, const FRoundResult& B)
	{
		return A.TimeSeconds < B.TimeSeconds;
	});

	ResultsBox->ClearChildren();

	static const TCHAR* Medals[] = { TEXT("1st"), TEXT("2nd"), TEXT("3rd") };

	for (int32 i = 0; i < Sorted.Num(); ++i)
	{
		const FRoundResult& R = Sorted[i];

		const int32 Min  = FMath::FloorToInt(R.TimeSeconds / 60.f);
		const int32 Sec  = FMath::FloorToInt(FMath::Fmod(R.TimeSeconds, 60.f));
		const int32 Cs   = FMath::FloorToInt(FMath::Fmod(R.TimeSeconds, 1.f) * 100.f);

		const FString Label = FString::Printf(
			TEXT("%s  %s  —  %02d:%02d.%02d"),
			(i < 3 ? Medals[i] : TEXT("   ")),
			*R.PlayerName,
			Min, Sec, Cs);

		ResultsBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(0.f, 0.f, 0.f, 12.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Label))
				.Font(FCoreStyle::GetDefaultFontStyle(i == 0 ? "Bold" : "Regular", 22))
				.ColorAndOpacity(White)
			];
	}
}
