// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "LapDogsApisTypes.h"
#include "ShibAPIsSubsystem.h"
#include "Metaverse/MetaverseApisTypes.h"
#include "LapDogsApisSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGetGameStatsDelegate, FGetGameStatsResponse, GetGameStatsResponse, bool, bSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSendGameStatsDelegate, bool, bSuccessful);

/**
 * 
 */
UCLASS()
class SHIBAPIS_API ULapDogsApisSubsystem : public UShibAPIsSubsystem
{
	GENERATED_BODY()

public:
	/**
	 *  LD LOGGING
	 */
	FString LdLoggingEndpoint = FString("/api/LD/INSERTLOG");
	UFUNCTION(BlueprintCallable, Category = "Requests|Logs")
	void Log(const FShibLog& Log);
	
	/**
	 * SEND GAME STATS
	 */
	FString SendGameStatsEndpoint = FString("/api/LD/ENDGAME");
	UFUNCTION(BlueprintCallable, Category = "Requests|GameStats")
	void SendGameStats(FSendGameStatsRequest SendGameStatsRequest);
	UFUNCTION()
	void OnSendGameStats(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|GameStats|Delegates")
	FOnSendGameStatsDelegate OnSendGameStatsDelegate;

	/**
	 * GET GAME STATS
	 */
	FString GetGameStatsEndpoint = FString("/api/LD/VIEWSTAT");
	UFUNCTION(BlueprintCallable, Category = "Requests|GameStats")
	void GetGameStats();
	UFUNCTION()
	void OnGetGameStats(bool bSuccessful, int32 Status, const FString& ResponseAsString);
	UPROPERTY(BlueprintAssignable, Category = "Requests|GameStats|Delegates")
	FOnGetGameStatsDelegate OnGetGameStatsDelegate;
};
