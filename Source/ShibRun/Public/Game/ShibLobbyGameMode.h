// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBaseGameMode.h"
#include "ShibMatchmakingEOS.h"
#include "ShibLobbyGameMode.generated.h"

UENUM(BlueprintType)
enum class ELobbyStatus : uint8
{
	// Waiting for all players to join the lobby
	WaitingForPlayers,

	// Waiting for all players to be ready before starting the race
	WaitingToStart,

	// Everyone is ready and the race is starting soon
	RaceIsStarting,
};

class AShibLobbyPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceReadyToStart, bool, bRaceReadyToStart);

UCLASS()
class SHIBRUN_API AShibLobbyGameMode : public AShibBaseGameMode
{
	GENERATED_BODY()

public:
	// Lobby time before race starts (in seconds)
	// This value isn't used if we are using the matchmaking system with dedicated server.
	// When using the matchmaking system and dedicated server, the lobby time is equal to ReservationTimeoutSeconds value in the
	// class ADedicatedServerMatchmakingBeaconHost
	UPROPERTY(EditDefaultsOnly)
	int32 LobbyTime = 55;

	// Lobby countdown before starting the race when all players are ready (in seconds)
	UPROPERTY(EditDefaultsOnly)
	int32 LobbyCountdownTime = 5;
	
	// Update lobby game room for all connected controllers
	void UpdateConnectedCtrlGameRoom();

	// Update lobby game room for the specified player state
	void ForceUpdateGameRoom(AShibLobbyPlayerState* TargetPS);

	UFUNCTION(BlueprintCallable)
	bool IsAllPlayersReadyToStart();

	UFUNCTION(BlueprintCallable)
	void NotifyPlayerIsReadyToStart(AShibLobbyPlayerState* PSReady, bool bReady);
	
	// This event is fired on the server when all players are ready to start the race
	// Used for Listen Server ONLY!
	UPROPERTY(BlueprintAssignable)
	FOnRaceReadyToStart OnRaceReadyToStart;

protected:
	virtual void BeginPlay() override;
	
	virtual void OnPostLogin(AController* NewPlayer) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION()
	void RaceReadyToStart(bool bIsReady);

	UFUNCTION()
	void StartSession();

	// Clear all timers at once to ensure that no timer is still running
	virtual void ClearGameTimers() override;

private:
	// Main match timer used for every match state
	void LobbyTimer();

	UFUNCTION()
	void HandleMatchmakingCompleted(const FString& TeamResults, const FShibMatchmakerHostConfiguration& Request);

	UFUNCTION()
	void HandleOnMatchReadyToStart();
	
	FTimerHandle StartingRaceTimerHandle;
	FTimerHandle LobbyTimerHandle;
	
	float LobbyTimeCounter=-1.f;
	ELobbyStatus LobbyStatus=ELobbyStatus::WaitingForPlayers;
};
