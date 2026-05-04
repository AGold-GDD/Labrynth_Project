#include "Lab_PlayerController.h"
#include "Lab_PlayerState.h"
#include "Lab_GameState.h"
#include "Lab_GameMode.h"
#include "Lab_GameInstance.h"
#include "Components/InputComponent.h"

void ALab_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	// Send username to server
	FString Username;
	if (ULab_GameInstance* GI = GetGameInstance<ULab_GameInstance>())
		Username = GI->GetLocalUsername();
	Server_SetDisplayName(Username.IsEmpty() ? TEXT("Player") : Username);

	// Lock movement immediately; it unlocks when InProgress begins
	if (ALab_GameState* GS = GetWorld()->GetGameState<ALab_GameState>())
	{
		GS->OnGamePhaseChanged.AddDynamic(this, &ALab_PlayerController::HandlePhaseChanged);
		HandlePhaseChanged(GS->GamePhase);
	}
}

void ALab_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Use Enter to ready up — avoids conflicting with the monster's E-to-tag chord.
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Enter, EInputEvent::IE_Pressed,
		                        this, &ALab_PlayerController::OnReadyPressed);
	}
}

void ALab_PlayerController::ReceivedGameState()
{
	Super::ReceivedGameState();

	if (!IsLocalController()) return;

	if (ALab_GameState* GS = GetWorld()->GetGameState<ALab_GameState>())
	{
		if (!GS->OnGamePhaseChanged.IsAlreadyBound(this, &ALab_PlayerController::HandlePhaseChanged))
			GS->OnGamePhaseChanged.AddDynamic(this, &ALab_PlayerController::HandlePhaseChanged);
		HandlePhaseChanged(GS->GamePhase);
	}
}

void ALab_PlayerController::HandlePhaseChanged(EGamePhase NewPhase)
{
	const bool bLocked = (NewPhase != EGamePhase::InProgress);
	if (bLocked)
	{
		SetIgnoreMoveInput(true);
		SetIgnoreLookInput(true);
	}
	else
	{
		ResetIgnoreMoveInput();
		ResetIgnoreLookInput(); // Must reset the counter fully — SetIgnoreLookInput(false) only decrements by 1
		EnableGameInputMode();  // Re-capture the mouse; UIOnly input mode (set during lobby) blocks mouse delta
	}
}

void ALab_PlayerController::OnReadyPressed()
{
	// Only meaningful during WaitingToStart; ignore during gameplay
	if (ALab_GameState* GS = GetWorld()->GetGameState<ALab_GameState>())
	{
		if (GS->GamePhase == EGamePhase::WaitingToStart)
		{
			Server_SetReady();
		}
	}
}

void ALab_PlayerController::Server_SetReady_Implementation()
{
	ALab_PlayerState* PS = GetPlayerState<ALab_PlayerState>();
	if (!PS || PS->bIsReady) return; // Already ready — ignore repeat presses

	PS->SetReady();

	if (ALab_GameMode* GM = GetWorld()->GetAuthGameMode<ALab_GameMode>())
	{
		GM->NotifyPlayerReady();
	}
}

void ALab_PlayerController::Server_SetDisplayName_Implementation(const FString& Name)
{
	if (ALab_PlayerState* PS = GetPlayerState<ALab_PlayerState>())
	{
		PS->SetDisplayName(Name);
	}
}

void ALab_PlayerController::EnableUIInputMode()
{
	FInputModeUIOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = true;
}

void ALab_PlayerController::EnableGameInputMode()
{
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;
}

bool ALab_PlayerController::IsMonsterPlayer() const
{
	const ALab_PlayerState* PS = GetPlayerState<ALab_PlayerState>();
	return PS && PS->PlayerRole == EPlayerRole::Monster;
}

bool ALab_PlayerController::IsLocalController_BP() const
{
	return IsLocalController();
}
