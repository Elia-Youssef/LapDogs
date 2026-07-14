// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibTypes.generated.h"

/**
 * Shib classes enum
 */
UENUM(BlueprintType)
enum class EShibClass : uint8
{
	GoodBoi_Class UMETA(DisplayName = "Good Boi"),
	ZoomyBoi_Class UMETA(DisplayName = "Zoomy Boi"),
	ChonkyBoi_Class UMETA(DisplayName = "Chonky Boi"),
	CoolBoi_Class UMETA(DisplayName = "Cool Boi"),
};

/**
* Struct that holds player data for the leaderboard
*/
USTRUCT(BlueprintType)
struct FLeaderboardRow
{
	GENERATED_BODY()

	FLeaderboardRow(){};
	
	FLeaderboardRow(const FUniqueNetIdRepl InPlayerId, const int32 InShibUserId, const FString& InPlayerName, const EShibClass InDogClass, const bool bInIsABot,
		const int32 InCurrentLap=0, const float InDistanceRan=0.f, const int32 InPlayerPosition=0)
	{
		PlayerId = InPlayerId;
		ShibUserId = InShibUserId;
		PlayerName = InPlayerName;
		DogClass = InDogClass;
		bIsABot = bInIsABot;
		CurrentLap = InCurrentLap;
		DistanceRan = InDistanceRan;
		PlayerPosition = InPlayerPosition;
	}

	UPROPERTY(BlueprintReadOnly)
	FUniqueNetIdRepl PlayerId = FUniqueNetIdRepl();
	
	UPROPERTY(BlueprintReadOnly)
	int32 ShibUserId = -1;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerName = FString();

	UPROPERTY(BlueprintReadOnly)
	EShibClass DogClass = EShibClass::GoodBoi_Class;

	UPROPERTY(BlueprintReadOnly)
	bool bIsABot = false;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentLap = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 PlayerPosition = 0;

	UPROPERTY(BlueprintReadOnly)
	float DistanceRan = 0.f;
	
	UPROPERTY(BlueprintReadOnly)
	int32 TumbledShibs = 0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 PickupItemsUsed = 0;

	UPROPERTY(BlueprintReadOnly)
	bool bRaceFinished = false;
	
	UPROPERTY(BlueprintReadOnly)
	TArray<float> LapsTime = {};

	UPROPERTY(BlueprintReadOnly)
	float RaceTimeInSeconds = 0.f; 
	
	UPROPERTY(BlueprintReadOnly)
	int32 Points = 0;

	bool operator==(const FLeaderboardRow& OtherItem) const
	{
		return PlayerId == OtherItem.PlayerId;
	}

	bool operator<(const FLeaderboardRow& OtherItem) const
	{
		// Primary indicator is the Race time, but Race time could potentially be not set (during the race)
		// So we use different comparison variables as well
		
		if (RaceTimeInSeconds > 0.f && OtherItem.RaceTimeInSeconds > 0.f)
		{
			return RaceTimeInSeconds < OtherItem.RaceTimeInSeconds;
		}

		if (CurrentLap != OtherItem.CurrentLap)
		{
			return CurrentLap > OtherItem.CurrentLap; // Higher lap = Higher in the leaderboard which means: This<OtherItem = true
		}
		
		if (DistanceRan != OtherItem.DistanceRan)
		{
			return DistanceRan > OtherItem.DistanceRan; // Bigger distance = Higher in the leaderboard which means: This<OtherItem = true
		}
		
		return PlayerPosition < OtherItem.PlayerPosition;
	}
};

/**
 * Struct that holds player data for Grand Prix
 */
USTRUCT(BlueprintType)
struct FGrandPrixRow
{
	GENERATED_BODY()

	FGrandPrixRow(){};
	
	FGrandPrixRow(const int32 InShibUserId, FUniqueNetIdRepl InPlayerId)
	{
		ShibUserId = InShibUserId;
		PlayerId = InPlayerId;
	};

	UPROPERTY(BlueprintReadOnly)
	int32 ShibUserId = -1;

	UPROPERTY(BlueprintReadOnly)
	FUniqueNetIdRepl PlayerId = FUniqueNetIdRepl();

	UPROPERTY(BlueprintReadOnly)
	int32 PlayerPosition=0;
	
	UPROPERTY(BlueprintReadOnly)
	int32 TotalPoints=0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FLeaderboardRow> RaceRounds;

	bool operator==(const FGrandPrixRow& OtherItem) const
	{
		// AIs won't have a valid ShibUserId
		// So we use PlayerId as a second option to compare
		if (ShibUserId != -1 && OtherItem.ShibUserId != -1)
		{
			return ShibUserId == OtherItem.ShibUserId;
		}

		return PlayerId == OtherItem.PlayerId;
	}

	bool operator<(const FGrandPrixRow& OtherItem) const
	{
		if (RaceRounds.IsEmpty() && !OtherItem.RaceRounds.IsEmpty()) return true;
		if (!RaceRounds.IsEmpty() && OtherItem.RaceRounds.IsEmpty()) return false;
		
		if (TotalPoints != OtherItem.TotalPoints)
		{
			return TotalPoints > OtherItem.TotalPoints;
		}
		
		float SelfBestLapTime=RaceRounds[0].LapsTime[0];
		float OtherBestLapTime=OtherItem.RaceRounds[0].LapsTime[0];
		// Tie-breaker for choosing which player is first in the leaderboard
		// Only for visual representation!
		for(int32 i=0; i < RaceRounds.Num(); i++)
		{
			for (auto SelfLapTime : RaceRounds[i].LapsTime)
			{
				if (SelfLapTime < SelfBestLapTime) SelfBestLapTime = SelfLapTime;
			}
			
			if (OtherItem.RaceRounds.IsValidIndex(i))
			{
				for (auto OtherLapTime : OtherItem.RaceRounds[i].LapsTime)
				{
					if (OtherLapTime < OtherBestLapTime) OtherBestLapTime = OtherLapTime;
				}
			}
			else
			{
				return false;
			}
		}
		return SelfBestLapTime < OtherBestLapTime; // Biggest lap time = lower rank in the leaderboard
	}

	void AddRaceRound(const FLeaderboardRow NewRound)
	{
		RaceRounds.Add(NewRound);
		//Increment total points
		int32 Points=0;
		for (auto Row : RaceRounds)
		{
			Points+=Row.Points;
		}
		TotalPoints=Points;
		PlayerPosition=0;// Reset player position
	}

	bool GetRoundLeaderboard(const int32 RoundNumb, FLeaderboardRow& LeaderboardFound)
	{
		if (RaceRounds.IsValidIndex(RoundNumb-1))
		{
			LeaderboardFound = RaceRounds[RoundNumb-1];
			return true;
		}
		return false;
	}
};

/**
 * Struct that holds Grand Prix rounds data
 */
USTRUCT(BlueprintType)
struct FGrandPrixLeaderboard
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 CompletedRoundsCount = 0;

	UPROPERTY(BlueprintReadOnly)
	TArray<FGrandPrixRow> GrandPrixRows;
};

UENUM(BlueprintType)
enum EShibJoinSessionResultType : uint8
{
	/** The join worked as expected */
	ResultSuccess  UMETA(DisplayName = "ResultSuccess"),
	/** There are no open slots to join */
	SessionIsFull UMETA(DisplayName = "Full"),
	/** The session couldn't be found on the service */
	SessionDoesNotExist UMETA(DisplayName = "DoesNotExist"),
	/** There was an error getting the session server's address */
	CouldNotRetrieveAddress UMETA(DisplayName = "CouldNotRetrieveAddress"),
	/** The user attempting to join is already a member of the session */
	AlreadyInSession UMETA(DisplayName = "AlreadyIn"),
	/** An error not covered above occurred */
	UnknownError UMETA(DisplayName = "UnknownError")
};

UENUM(BlueprintType)
enum EShibSearchSessionType : uint8
{
	/* Search for public and private sessions */
	ST_All UMETA(DisplayName = "All"),
	/* Search for private sessions */
	ST_Private UMETA(DisplayName = "Private"),
	/* Search for public sessions */
	ST_Public UMETA(DisplayName = "Public"),
};

/**
 * Use for types related functions
 */
UCLASS()
class SHIBRUN_API UShibTypes : public UObject
{
	GENERATED_BODY()

public:
};
