// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBaseGameMode.h"
#include "LapDogs/LapDogsApisTypes.h"
#include "ShibGameMode.generated.h"

class AShibAiController;
class AFixedSpectatorCamera;
struct FLeaderboardRow;
class ARouteSpline;
class AShibController;

/**
 * 
 */
UCLASS()
class SHIBRUN_API AShibGameMode : public AShibBaseGameMode
{
	GENERATED_BODY()
	
public:
	virtual void StartPlay() override;
	virtual void Logout(AController* Exiting) override;
	virtual void PreLogout(APlayerController* InPlayerController) override;

	// We override this function because we have more match state types than the base Game Mode Class.
	// We trigger different events depending on the current match state.
	virtual void OnMatchStateSet() override;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateLeaderboardWidget(const TArray<FLeaderboardRow>& Leaderboard);

	// Function called by each player when they are fully setup and ready to race
	// The race countdown will start after all the connected players called this function
	UFUNCTION(BlueprintCallable)
	void PlayerIsReadyToRace();

	// Called by the Game State to notify the Game Mode that a player finished the race
	// Game Mode will fill the rest of the stats of the player here and trigger events accordingly
	UFUNCTION(BlueprintCallable)
	void PlayerFinishedRace(AController* Controller, FLeaderboardRow& PlayerStats);

	// Return the time at which the match started
	UFUNCTION(BlueprintCallable)
	FDateTime GetMatchStartTime() { return MatchStartTime; }

	// Maximum time we wait for players to join the race
	// if one player didn't join after this time, we kick him out of the current session
	UPROPERTY(EditDefaultsOnly)
	int32 WaitingForPlayersMatchTime = 30;
	
	// Countdown before match starts (in seconds)
	UPROPERTY(EditDefaultsOnly)
	int32 CountdownMatchTime = 5;

	// Main Match Time (in seconds)
	// 1 Hour equal 3600 seconds
	UPROPERTY(EditDefaultsOnly)
	int32 MatchTime = 3600;
	
	// Delay before ending the race when the first player cross the finish line (in seconds)
	UPROPERTY(EditDefaultsOnly)
	int32 PostMatchTime = 30;

	// Delay before moving all the players to the next level when the race ends (in seconds)
	// This is when all the players see the final leaderboard
	UPROPERTY(EditDefaultsOnly)
	int32 EndMatchTime = 20;

	// Temp solution (maybe not)
	// How much Coins the first player of the race will be awarded
	// Each consequent player will be awarded one point less per position
	UPROPERTY(EditDefaultsOnly)
	int32 FirstPlayerAwardedPoints = 12;
	
	// Update the spectator camera every x seconds
	UPROPERTY(EditDefaultsOnly)
	float SpectatorCameraUpdateRate = 0.5;

protected:
	virtual void BeginPlay() override;
	
	// Handle functions for each match state we want to do something special
	virtual void HandleMatchCountdown();
	virtual void HandleMatchIsWaitingToStart() override;
	virtual void HandleMatchHasStarted() override;
	virtual void HandleMatchEndPending();
	virtual void HandleMatchHasEnded() override;

	// Clear all timers at once to ensure that no timer is still running
	virtual void ClearGameTimers() override;

	// Handle any new players joining the session
	// Used to restart players' pawn
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	// Main match timer used for every match state
	void MatchTimer();
	
	// Notify players when match state changes
	void NotifyCurrentMatchState();

	// Verify if all players are ready and we can start the race
	bool IsAllPlayersReadyToRace();

	// Verify if the current session is full (players and AIs included)
	// Fill all empty slots with AIs
	void FillSessionWithAIs();

	// Internal callback after sending the race stats to the Shib APIs when the race is over
	UFUNCTION()
	void HandleSendGameStatsComplete(bool bSuccessful);

	// Race stats request which we keep in memory in case we need to send it again to the Shib APIs if something goes wrong.
	FSendGameStatsRequest CachedRaceResultsRequest;
	
	FTimerHandle WaitingForPLayersTimerHandle;
	FTimerHandle CountdownTimerHandle;
	FTimerHandle MatchTimerHandle;
	FTimerHandle EndPendingTimerHandle;
	FTimerHandle EndTimerHandle;
	FTimerHandle SpectatorCamerasHandle;

	FDateTime MatchStartTime;
	float MatchTimeCounter;
};
