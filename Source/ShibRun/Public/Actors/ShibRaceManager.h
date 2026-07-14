// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Info.h"
#include "Utils/ShibTypes.h"
#include "ShibRaceManager.generated.h"

class AShibAiController;
class UGrandPrixAsset;
/**
 * Singleton actor that is spawned in every race map
 * It allows us to make the races data persistent between levels when playing a Grand Prix.
 */
UCLASS(Blueprintable)
class SHIBRUN_API AShibRaceManager : public AInfo
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShibRaceManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Setup custom replicated variables
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > &OutLifetimeProps) const override;

	/**
	 * Get the complete Grand Prix leaderbaord
	 * @return Grand Prix Leaderboard
	 */
	UFUNCTION(BlueprintGetter, Category=ShibRaceManager)
	FGrandPrixLeaderboard GetGrandPrixLeaderboard() { return GrandPrixLeaderboard; }

	/**
	 * Get the next Grand Prix level to play based on the amount of rounds we did.
	 * @param NextLevel 
	 * @return true with the level reference if the GP isn't finish yet and we have more race to do.
	 * It will return false when the GP is over.
	 */
	UFUNCTION(Category=ShibRaceManager)
	bool GetNextGrandPrixLevel(TSoftObjectPtr<UWorld>& NextLevel) const;

	/**
	 * Get the current Grand Prix asset we're currently playing.
	 * This asset defines all the rules of the Grand Prix including the amount of rounds and the levels
	 * @return Current Grand Prix Asset
	 */
	UFUNCTION(BlueprintGetter, Category=ShibRaceManager)
	UGrandPrixAsset* GetCurrentGrandPrixAsset() { return CurrentGrandPrix; }

	/**
	 * Set the current Grand Prix we're playing
	 * @param GrandPrixId The id of the Grand Prix. The id is defined in each Grand Prix Data Asset
	 * @return True if successful
	 */
	UFUNCTION(BlueprintCallable, Category=ShibRaceManager)
	bool SetCurrentGrandPrixAsset(const FString& GrandPrixId);

	/**
	 * Get a specific Grand Prix row in the Grand Prix leaderboard
	 * A row contains the player's stats of the Grand Prix
	 * AIs stats aren't accessible from this function, the function needs to rece
	 * @param GrandPrixRowFound Grand Prix Row found returned by this function
	 * @param ShibUserId Shib User Id of the player (Optional if we pass a valid PlayerId)
	 * @param PlayerId Unique Net Id of the player (Optional if we pass a valid ShibUserId)
	 * @return True if we found the Grand Prix row.
	 */
	UFUNCTION(BlueprintCallable, Category=ShibRaceManager)
	bool GetGrandPrixRow(FGrandPrixRow& GrandPrixRowFound, const int32 ShibUserId=-1, const FUniqueNetIdRepl PlayerId=FUniqueNetIdRepl());

	/**
	 * Save the final race leaderbaord after a race in the Grand Prix Leaderboard
	 * @param Leaderboard The final race leaderboard
	 */
	UFUNCTION(Category=ShibRaceManager)
	void AddGrandPrixRound(const TArray<FLeaderboardRow>& Leaderboard);

	/**
	 * Get the amount of round we completed until now
	 * @return Completed rounds count
	 */
	UFUNCTION(BlueprintPure, Category=ShibRaceManager)
	int32 GetCompletedGrandPrixRoundsCounts() const { return GrandPrixLeaderboard.CompletedRoundsCount; }

	/**
	 * Get the total amount of rounds for the current Grand Prix
	 * @return Total GP rounds, will return -1 if we can't find this information
	 */
	UFUNCTION(BlueprintPure, Category=ShibRaceManager)
	int32 GetTotalGrandPrixRounds() const;

	/**
	 * Get the number of player we're expecting to receive in the current session
	 * @return The number of player we expect to have in the current session
	 */
	UFUNCTION(BlueprintPure, Category=ShibRaceManager)
	int32 GetExpectedNumbOfPlayers() const { return ExpectedNumbOfPlayers; }

	/**
	 * Set the amount of player we expect to reveive for the current session
	 * @param NumbOfPlayers Number of players currently in the session
	 */
	void SetExpectedNumbOfPlayers(const int32 NumbOfPlayers);

	// All the available Grand Prix players can play
	UPROPERTY(EditDefaultsOnly, Category=ShibRaceManagerSettings)
	TArray<UGrandPrixAsset*> GrandPrixList;

	// Weather or not the game mode should spawn AIs to fill empty slots
	UPROPERTY(EditDefaultsOnly, Category="ShibRaceManagerSettings|Ai")
	bool bSpawnAI = true;

	// The list of AI controllers spawned by the game mode
	UPROPERTY(BlueprintReadOnly, Category="ShibRaceManagerSettings|Ai")
	TArray<AShibAiController*> AIControllers;

private:
	// The current Grand Prix leaderboard that contains all the races stats for each participant
	UPROPERTY(Replicated, BlueprintGetter=GetGrandPrixLeaderboard)
	FGrandPrixLeaderboard GrandPrixLeaderboard;

	// The current Grand Prix we're currently playing
	UPROPERTY(ReplicatedUsing=OnRep_CurrentGrandPrix, BlueprintGetter=GetCurrentGrandPrixAsset)
	UGrandPrixAsset* CurrentGrandPrix=nullptr;
	
	// Number of players we expect to receive in the game session
	int32 ExpectedNumbOfPlayers=1;
	
	UFUNCTION()
	void OnRep_CurrentGrandPrix();
};
