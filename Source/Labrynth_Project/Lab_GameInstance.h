#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Lab_GameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostResult, bool, bWasSuccessful);

/**
 * ULab_GameInstance
 *
 * Persists across level loads (unlike GameMode/GameState which reset per level).
 * Handles Online Subsystem session creation so WBP_Menu doesn't need to wire
 * the low-level Create Session / Destroy Session nodes manually.
 *
 * Blueprint usage (in WBP_Menu or Lobby Level Blueprint):
 *   Get Game Instance > Cast to BP_Lab_GameInstance
 *   Call HostGame or JoinGameByIP
 *   Bind to OnHostSessionResult to react to success/failure
 *
 * Set BP_Lab_GameInstance as the Game Instance class in:
 *   Project Settings > Maps & Modes > Game Instance Class
 */
UCLASS()
class LABRYNTH_PROJECT_API ULab_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	ULab_GameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	// ── Host ─────────────────────────────────────────────────────────────────

	// Creates an Online Subsystem session then server-travels to TravelMapPath with ?listen.
	// TravelMapPath example: "/Game/MultiplayerStuff/MazeLevel"
	// MaxPlayers should match MaxPlayers in DefaultGame.ini (currently 3).
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void HostGame(const FString& TravelMapPath, int32 MaxPlayers = 3);

	// Fired when the session is created (bWasSuccessful = true) or fails (false).
	// Bind to this in WBP_Menu to show an error message on failure.
	UPROPERTY(BlueprintAssignable, Category = "Multiplayer")
	FOnHostResult OnHostSessionResult;

	// ── Join ─────────────────────────────────────────────────────────────────

	// Connects to the host at IPAddress on port 7777.
	// IPAddress should be a plain IPv4 string: "192.168.1.42" or "203.0.113.77"
	// The engine appends the port automatically.
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void JoinGameByIP(const FString& IPAddress);

	// ── Utilities ────────────────────────────────────────────────────────────

	// Returns the machine's local IPv4 address as a string (e.g. "192.168.1.42").
	// Returns "Unavailable" if the socket subsystem cannot determine it.
	// Show this in WBP_Menu after the host clicks HOST GAME so teammates know what to type.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Multiplayer")
	FString GetLocalIPAddress() const;

	// Destroys the current session (call on return to main menu / game over).
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void DestroyCurrentSession();

	// ── Random map pool ──────────────────────────────────────────────────────────

	// Fill this in BP_Lab_GameInstance's Class Defaults with map paths to randomly
	// pick from. Example: "/Game/MultiplayerStuff/lvl1"
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Multiplayer")
	TArray<FString> MapPool;

	// Picks a random map from MapPool and calls HostGame with it.
	// Add at least one entry to MapPool in BP_Lab_GameInstance's Class Defaults.
	UFUNCTION(BlueprintCallable, Category = "Multiplayer")
	void HostGameFromPool(int32 MaxPlayers = 3);

	// ── Username ─────────────────────────────────────────────────────────────────

	// The name this local player typed in the menu. Persists across level loads.
	// Stored in GameInstance so it survives seamless travel to the game map.
	UPROPERTY(BlueprintReadOnly, Category = "Player")
	FString LocalUsername;

	// Called by WBP_Menu when the player confirms their name. Already called
	// automatically when they click HOST or JOIN — no manual wiring needed.
	UFUNCTION(BlueprintCallable, Category = "Player")
	void SetLocalUsername(const FString& Name);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player")
	FString GetLocalUsername() const;

private:
	IOnlineSessionPtr SessionInterface;
	FString PendingTravelMap;

	FDelegateHandle OnCreateSessionHandle;
	FDelegateHandle OnDestroySessionHandle;

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
};
