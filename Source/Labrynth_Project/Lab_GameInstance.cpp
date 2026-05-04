#include "Lab_GameInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "GameFramework/PlayerController.h"
#include "SocketSubsystem.h"

ULab_GameInstance::ULab_GameInstance() {}

void ULab_GameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	if (OSS)
	{
		SessionInterface = OSS->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			OnCreateSessionHandle = SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
				this, &ULab_GameInstance::HandleCreateSessionComplete);

			OnDestroySessionHandle = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
				this, &ULab_GameInstance::HandleDestroySessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("ULab_GameInstance: No Online Subsystem found. "
		            "HostGame will fall back to direct server travel."));
	}
}

void ULab_GameInstance::Shutdown()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->OnCreateSessionCompleteDelegates.Remove(OnCreateSessionHandle);
		SessionInterface->OnDestroySessionCompleteDelegates.Remove(OnDestroySessionHandle);
	}
	Super::Shutdown();
}

void ULab_GameInstance::HostGame(const FString& TravelMapPath, int32 MaxPlayers)
{
	SessionPlayerCount = FMath::Clamp(MaxPlayers, 3, 6);
	PendingTravelMap = TravelMapPath;

	// If no session interface is available (e.g. NULL subsystem not fully initialized),
	// skip session creation and travel directly. This is fine for LAN testing.
	if (!SessionInterface.IsValid())
	{
		FString URL = TravelMapPath + TEXT("?listen");
		GetWorld()->ServerTravel(URL);
		return;
	}

	// Destroy any existing session first (e.g. if the host returns to the menu and re-hosts).
	// The DestroySession callback will NOT re-call HostGame; call HostGame again manually.
	// For a simple flow where the host only ever hosts once, this is not needed.
	if (SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
		// HandleDestroySessionComplete will log that a session was destroyed,
		// but we intentionally don't re-host automatically here to keep control explicit.
	}

	FOnlineSessionSettings Settings;
	Settings.NumPublicConnections = MaxPlayers;
	Settings.NumPrivateConnections = 0;

	// bIsLANMatch should be true when using the NULL subsystem (no real backend).
	// When you switch to Steam or EOS, set this to false.
	const bool bUsingNullSubsystem = (IOnlineSubsystem::Get()->GetSubsystemName() == TEXT("NULL"));
	Settings.bIsLANMatch = bUsingNullSubsystem;

	Settings.bUsesPresence       = !bUsingNullSubsystem; // Presence = Steam/EOS friend invites
	Settings.bShouldAdvertise    = true;
	Settings.bAllowJoinInProgress = false; // Prevent players connecting after the game starts
	Settings.bAllowJoinViaPresence = !bUsingNullSubsystem;
	Settings.bUseLobbiesIfAvailable = !bUsingNullSubsystem;

	SessionInterface->CreateSession(0, NAME_GameSession, Settings);
}

void ULab_GameInstance::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	OnHostSessionResult.Broadcast(bWasSuccessful);

	if (bWasSuccessful)
	{
		FString URL = PendingTravelMap + TEXT("?listen");
		GetWorld()->ServerTravel(URL);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ULab_GameInstance: CreateSession failed for '%s'."), *SessionName.ToString());
	}
}

void ULab_GameInstance::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("ULab_GameInstance: Session '%s' destroyed (success=%d)."),
	       *SessionName.ToString(), bWasSuccessful);
}

void ULab_GameInstance::JoinGameByIP(const FString& IPAddress)
{
	APlayerController* PC = GetFirstLocalPlayerController();
	if (PC)
	{
		// ClientTravel connects this client to the server at the given address.
		// TRAVEL_Absolute means the string is a full address, not relative.
		PC->ClientTravel(IPAddress, ETravelType::TRAVEL_Absolute);
	}
}

void ULab_GameInstance::DestroyCurrentSession()
{
	if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession))
	{
		SessionInterface->DestroySession(NAME_GameSession);
	}
}

void ULab_GameInstance::HostGameFromPool(int32 MaxPlayers)
{
	if (MapPool.IsEmpty())
	{
		UE_LOG(LogTemp, Warning,
		       TEXT("HostGameFromPool: MapPool is empty. "
		            "Add map paths to BP_Lab_GameInstance's MapPool in Class Defaults."));
		return;
	}

	const int32 Index = FMath::RandRange(0, MapPool.Num() - 1);
	HostGame(MapPool[Index], MaxPlayers);
}

void ULab_GameInstance::SetLocalUsername(const FString& Name)
{
	LocalUsername = Name;
}

FString ULab_GameInstance::GetLocalUsername() const
{
	return LocalUsername;
}

FString ULab_GameInstance::GetLocalIPAddress() const
{
	ISocketSubsystem* SocketSS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
	if (!SocketSS) return TEXT("Unavailable");

	bool bCanBind = false;
	TSharedRef<FInternetAddr> Addr = SocketSS->GetLocalHostAddr(*GLog, bCanBind);
	if (bCanBind)
	{
		return Addr->ToString(false); // false = don't append port number
	}
	return TEXT("Unavailable");
}
