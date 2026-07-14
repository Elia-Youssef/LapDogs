// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBaseGameState.h"
#include "Utils/ShibTypes.h"
#include "ShibGameState.generated.h"

struct FGameplayTag;
class AShibPlayerState;
class ARouteSpline;
class AShibController;
class AShibGameMode;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceLeaderboardUpdateDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceLapFinishDelegate, int32, CurrentLap);

UCLASS()
class SHIBRUN_API AShibGameState : public AShibBaseGameState
{
	GENERATED_BODY()

public:

	// Setup custom replicated variables
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > &OutLifetimeProps) const override;

	// Get the current leaderboard
	UFUNCTION(Category = "ShibGameState|Leaderboard", BlueprintGetter)
	TArray<FLeaderboardRow> GetCurrentLeaderboard() { return RaceLeaderboard; }
	
	// Get the final leaderboard
	UFUNCTION(Category = "ShibGameState|Leaderboard", BlueprintGetter)
	TArray<FLeaderboardRow> GetFinalLeaderboard() { return FinalLeaderboard; }
	
	// Find and get a specific leaderboard row based on the player Id.
	// Return true with the Leaderboard row reference if the row is found.
	UFUNCTION(BlueprintCallable, Category = "ShibGameState|Leaderboard")
	bool GetLeaderboardRowByPlayerNetId(FUniqueNetIdRepl PlayerId, FLeaderboardRow& LeaderboardRowFound, const bool bFinal = false);

	// Find and get a specific leaderboard row index based on the player Id.
	// Return the leaderboard row index reference if the row is found.
	UFUNCTION(BlueprintCallable, Category = "ShibGameState|Leaderboard")
	int32 GetLeaderboardRowIndexByPlayerNetId(FUniqueNetIdRepl PlayerId, const bool bFinal = false);

	// Called when a player cross the finish line to update its status
	// This should be the only function called when a player cross the finish line
	UFUNCTION(BlueprintCallable)
	void PlayerCrossedFinishLine(APlayerState* PlayerStateRef, FDateTime FinishTime);

	// Start and stop the leaderboard update, called when the race start by the game mode
	UFUNCTION(BlueprintCallable)
	void StartLeaderboardUpdate();

	// Stop the leaderboard update, called when the race stop by the game mode
	UFUNCTION(BlueprintCallable)
	void StopLeaderboardUpdate();

	// Validate abd generate the leaderboard when the race ends.
	// This function will make sure all the players are added to the leaderboard
	bool ValidateAndGenerateFinalLeaderboard(TArray<FLeaderboardRow>& FinalRaceLeaderboard);

	// When we want to make sure every player in the race leaderboard is valid
	// Not currently use, but maybe this can be useful at some point
	void ValidateRaceLeaderboard();
	
	// Leaderboard update frequency (every x seconds)
	UPROPERTY(EditDefaultsOnly, Category = "ShibGameState|RaceSettings")
	float LeaderboardUpdateFrequency = 0.25f;
	
	// Temp solution
	// Max amount of laps of the race
	// In the future, this should be part of a data structure that is used to define the settings of each racetrack
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ShibGameState|RaceSettings")
	int32 RaceLaps = 3;

	// Current lap of the first player in the race
	// Mainly used to fires some events in the world during the race (visual)
	UPROPERTY(Transient, BlueprintReadOnly, Category = "ShibGameState|RaceSettings")
	int32 CurrentRaceLap=0;

	// Race manager singleton actor that persist between levels
	// This is where we store Grand Prix data
	UPROPERTY(BlueprintReadOnly, Replicated)
	TObjectPtr<AShibRaceManager> RaceManager;

	// Event fired when the leaderboard got updated
	UPROPERTY(BlueprintAssignable)
	FOnRaceLeaderboardUpdateDelegate OnRaceLeaderboardUpdate;
	
	// Event fired when the final leaderboard get updated
	UPROPERTY(BlueprintAssignable)
	FOnRaceLeaderboardUpdateDelegate OnFinalLeaderboardUpdate;

	// Event fired when a new lap has started for the first player in the race
	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnRaceLapFinishDelegate OnRaceLapFinish;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnRep_MatchState() override;

	// Update the race leaderboard at a constant frequency
	void UpdateLeaderboard();

private:
	FTimerHandle LeaderboardUpdateHandle;

	UPROPERTY(ReplicatedUsing=Onrep_RaceLeaderboard, BlueprintGetter=GetCurrentLeaderboard)
	TArray<FLeaderboardRow> RaceLeaderboard;

	UPROPERTY(ReplicatedUsing=OnRep_FinalLeaderboard, BlueprintGetter=GetFinalLeaderboard)
	TArray<FLeaderboardRow> FinalLeaderboard;

	// Record ability statistics throughout the race
	// This event will fire everytime a player fires an ability
	UFUNCTION()
	void RecordAbilityInLeaderboard(const FGameplayTag AbilityTag, const AActor* AbilityOwner, const AActor* AbilityInstigator, float ExecutionTime);

	UFUNCTION()
	void OnRep_RaceLeaderboard();

	UFUNCTION()
	void OnRep_FinalLeaderboard();

	// Spline actor placed along the track in the map used to track down current players' position
	UPROPERTY()
	TObjectPtr<ARouteSpline> TrackSpline;
	// Length of the Spline used to track down current players' position
	UPROPERTY()
	float TrackSplineLength;
};
