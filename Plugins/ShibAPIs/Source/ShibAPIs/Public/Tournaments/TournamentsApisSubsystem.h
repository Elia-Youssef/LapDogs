// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAPIsSubsystem.h"
#include "TournamentsApisTypes.h"
#include "TournamentsApisSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetTournaments, const TArray<FTournamentDetails>&, Tournaments);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetPlayerScore, FTournamentPlayer, Player);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetTournamentRewards, const TArray<FTournamentRewards>&, Rewards);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGetTournamentParticipants, const TArray<FTournamentPlayer>&, Participants);

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnTournamentResponse, bool, bResponse);

/**
 * 
 */
UCLASS()
class SHIBAPIS_API UTournamentsApisSubsystem : public UShibAPIsSubsystem
{
	GENERATED_BODY()

#pragma region HttpRequests

protected:
	virtual bool GetUrl(FString& Url) override;
	virtual TMap<FString, FString> GetAuth() override;
#pragma endregion HttpRequests

public:
	/**
	 * Is Tournament Valid
	 */
	FString IsTournamentValidEndpoint = FString("/validate_tournament");
	UFUNCTION(BlueprintCallable, Category = "Requests|Tournaments")
	void IsTournamentValid(FOnTournamentResponse Event, FString TournamentId);

	/**
	 * Is Tournament Active
	 */
	FString IsTournamentActiveEndpoint = FString("/check_active_tournament");
	UFUNCTION(BlueprintCallable, Category = "Requests|Tournaments")
	void IsTournamentActive(FOnTournamentResponse Event, FString TournamentId);

	/**
	 * Get Tournament Participants
	 */
	FString GetParticipantsEndpoint = FString("/get_participants");
	UFUNCTION(BlueprintCallable, Category = "Requests|Tournaments")
	void GetParticipants(FOnGetTournamentParticipants Event, FString TournamentId);

	/**
	 * Get Tournament Rewards
	 */
	FString GetRewardsEndpoint = FString("/get_rewards");
	UFUNCTION(BlueprintCallable, Category = "Requests|Tournaments")
	void GetRewards(FOnGetTournamentRewards Event, FString TournamentId);

	/**
	 * Set Player Score
	 */
	FString SetPlayerScoreEndpoint = FString("/set_player_score");
	UFUNCTION(BlueprintCallable, Category = "Requests|Tournaments")
	void SetPlayerScore(FOnTournamentResponse Event, FString TournamentId, FString Wallet, int32 Score);

	/**
	 * Get Player Score
	 */
	FString GetPlayerScoreEndpoint = FString("/get_player_score");
	UFUNCTION(BlueprintCallable, Category = "Requests|Tournaments")
	void GetPlayerScore(FOnGetPlayerScore Event, FString TournamentId, FString Wallet);

	/**
	 * Get Tournaments
	 */
	FString GetTournamentsEndpoint = FString("/get_tournaments_with_meta");
	UFUNCTION(BlueprintCallable, Category = "Requests|Tournaments")
	void GetTournaments(FOnGetTournaments Event);

	/**
	 * Get Tournaments With Metadata By Player
	 */
	FString GetTournamentsByPlayerEndpoint = FString("/get_tournaments_with_meta_by_player");
	UFUNCTION(BlueprintCallable, Category = "Requests|Tournaments")
	void GetTournamentsByPlayer(FOnGetTournaments Event, FString Wallet);

private:
	static TArray<FTournamentDetails> ParseTournamentsMapFromJson(const FString& Json);
};
