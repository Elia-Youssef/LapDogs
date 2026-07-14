// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBaseController.h"
#include "ShibController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMatchStateChangedDelegate, FName, CurrentMatchState, float, MatchStateTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRoundTripTimeUpdated);

class AShibGameMode;
class UShibMatchEndWidget;
struct FRound;

UCLASS()
class SHIBRUN_API AShibController : public AShibBaseController
{
	GENERATED_BODY()

public:
	virtual void ReceivedPlayer() override;
	
	/**
	* Returns the round trip time it takes for an rpc to go from the server
	* to the client.
	*/
	UFUNCTION(BlueprintCallable)
	virtual float GetServerRoundTripTime() { return ServerRoundTripTime; }
	
	// Local event triggered when the match state change on the server
	UPROPERTY(BlueprintAssignable)
	FOnMatchStateChangedDelegate OnMatchStateChanged;

	// Function called by the game mode when match state changes.
	// This is mainly used to trigger local events to let the players know the current match state.
	UFUNCTION(Client, Reliable)
	void Client_OnMatchStateChangeNotify(FName CurrentMatchState, float MatchStateTime=0.0f);

#pragma region Obstacles
	
	UFUNCTION(Client, Reliable)
	void Client_ObstacleChangeState(bool bNewState) const;
	
	TArray<AActor*> GetNearbyActorsOfClass(UClass* ActorClass, float Range) const;

#pragma endregion Obstacles

protected:
	virtual void BeginPlay() override;

	virtual void OnRep_PlayerState() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnRepPlayerState();
	
	/**
	* Reports the current round trip time to client in response
	* to ServerRequestServerTime
	*/
	UFUNCTION(Client, Reliable)
	void Client_ReportRoundTripTime(float ClientTime, float ServerTime);

	/**
	* Requests current server time so accurate lag
	* compensation can be performed in ClientReportServerTime
	* based on the round-trip duration
	* Do not try to access the variable ServerRoundTripTime right after calling this function
	* since this is an RPC and it can take some time before updating the value on the client side
	*/
	UFUNCTION(Server, Reliable, BlueprintCallable, WithValidation)
	void Server_RequestRoundTripTime(APlayerController* requester, float ClientTime);

	// All these functions below are used when there is a change Match State
	// Match State changes are only done by the Game Mode, we just react to those changes
	
	// Function triggered when the game match state change to Countdown
	UFUNCTION(BlueprintImplementableEvent)
	void OnRaceWarmup(float WarmupTime);

	// Function triggered when the game match state change to In Progress
	UFUNCTION(BlueprintImplementableEvent)
	void OnRaceStart(float MatchTime);

	// Function triggered when the game match state change to Pending End
	UFUNCTION(BlueprintImplementableEvent)
	void OnRacePendingEnd(float PendingEndTime);

	// Function triggered when the player cross the finish line
	UFUNCTION(BlueprintImplementableEvent)
	void OnPlayerFinishedRace();
	
	// Function triggered when the game match state change to Pending End
	UFUNCTION(BlueprintImplementableEvent)
	void OnRaceEnd();

	// Buffer list of round trip time to get a good sample of the RTT
	// We will use the median value out of this sample to get our RTT value;
	TArray<float> ServerRoundTripTimeBuffer;
	
	// The approx amount of time it takes for a client rpc from the server to reach the client.
	// Use for local timers to be more in sync with the server's timers.
	float ServerRoundTripTime = -1.f;

	UPROPERTY(BlueprintAssignable)
	FOnRoundTripTimeUpdated OnRoundTripTimeUpdatedDelegate;
};
