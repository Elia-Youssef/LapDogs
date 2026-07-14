// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "LapDogsApisTypes.generated.h"


USTRUCT(BlueprintType)
struct FPlayerStatsRequest
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 UserId = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 Position = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 XP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 Coin = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 Deaths = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Time = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 DogClass = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 distanceRunning = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool raceFinishes = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 TumbledOtherShibs = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 PickupItemsUsed = 0;
};

// Send Game Stats Request Body
USTRUCT(BlueprintType)
struct FSendGameStatsRequest
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString EosSession = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString MapTitle = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 LapNumber = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FPlayerStatsRequest> Players = TArray<FPlayerStatsRequest>();
};

USTRUCT(BlueprintType)
struct FShibPlayerDogClassTimePlayed
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 Class = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float TimePlayed = 0.f;
};

// LapDogs' Game Stats Player
USTRUCT(BlueprintType)
struct FShibStatsPlayer
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString Name = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 XP = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 Coins = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float TotalTimePlayed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 TotalDistanceRunning = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 TotalRaceFinishes = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 TumbledOtherShibs = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 PickupItemsUsed = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FShibPlayerDogClassTimePlayed> DogClassTimePlayed;
};

// Game Stats Actual Stats
USTRUCT(BlueprintType)
struct FShibStats
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 GameFinishTop3 = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float TimePlayed = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float BestRecord = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 DeathsInGames = 0;
};

// Game Stats Top 3 Records
USTRUCT(BlueprintType)
struct FShibStatsRecords
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString MapTitle = FString();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Time = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 Position = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 DogClassTitle = 0;
};

// Player's Game Stats
USTRUCT(BlueprintType)
struct FGetGameStatsResponse
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FShibStatsPlayer Player = FShibStatsPlayer();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FShibStats Stats = FShibStats();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<FShibStatsRecords> TopRecords = TArray<FShibStatsRecords>();
};


/**
 * 
 */
UCLASS()
class SHIBAPIS_API ULapDogsApisTypes : public UObject
{
	GENERATED_BODY()
};
