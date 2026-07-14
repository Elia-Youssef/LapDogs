// Copyright Shiba Inu Games LLC.

#include "Actors/ShibRaceManager.h"
#include "Game/Data/GrandPrixAsset.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

AShibRaceManager::AShibRaceManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AShibRaceManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS(AShibRaceManager, GrandPrixLeaderboard, Params);
	DOREPLIFETIME(AShibRaceManager, CurrentGrandPrix);
}

bool AShibRaceManager::GetNextGrandPrixLevel(TSoftObjectPtr<UWorld>& NextLevel) const
{
	if (CurrentGrandPrix)
	{
		if (CurrentGrandPrix->RandomTracks) // Get a random level in the list
		{
			// We don't want to return something if RoundCount is more or equal to the rounds number because it means the GP is over.
			if (!CurrentGrandPrix->RandomTracksList.IsEmpty() && GrandPrixLeaderboard.CompletedRoundsCount < CurrentGrandPrix->RaceRounds)
			{
				NextLevel = CurrentGrandPrix->RandomTracksList[FMath::RandRange(0, CurrentGrandPrix->RandomTracksList.Num()-1)];
				return true;
			}
		}
		else
		{
			// Get the next track in the list based on the amount of rounds we already did
			if (CurrentGrandPrix->GrandPrixTracks.IsValidIndex(GrandPrixLeaderboard.CompletedRoundsCount-1))
			{
				NextLevel = CurrentGrandPrix->GrandPrixTracks[GrandPrixLeaderboard.CompletedRoundsCount];
				return true;
			}
		}
	}
	return false;
}

bool AShibRaceManager::SetCurrentGrandPrixAsset(const FString& GrandPrixId)
{
	if (!HasAuthority()) return false;
	
	for (auto GP : GrandPrixList)
	{
		if (GP->GrandPrixId == GrandPrixId)
		{
			CurrentGrandPrix = GP;
			return true;
		}
	}
	return false;
}

bool AShibRaceManager::GetGrandPrixRow(FGrandPrixRow& GrandPrixRowFound, const int32 ShibUserId, const FUniqueNetIdRepl PlayerId)
{
	FGrandPrixRow RowToFind(ShibUserId, PlayerId);
	if (int32 RowIdx = GrandPrixLeaderboard.GrandPrixRows.Find(RowToFind); RowIdx != INDEX_NONE)
	{
		GrandPrixRowFound = GrandPrixLeaderboard.GrandPrixRows[RowIdx];
		return true;
	}
	
	return false;
}

void AShibRaceManager::AddGrandPrixRound(const TArray<FLeaderboardRow>& Leaderboard)
{
	if (!HasAuthority()) return;
	
	for (const auto LeaderboardRow : Leaderboard)
	{
		FGrandPrixRow NewGrandPrixRow(LeaderboardRow.ShibUserId, LeaderboardRow.PlayerId);

		if (int32 RowIdx = GrandPrixLeaderboard.GrandPrixRows.Find(NewGrandPrixRow); RowIdx!=INDEX_NONE)
		{
			GrandPrixLeaderboard.GrandPrixRows[RowIdx].AddRaceRound(LeaderboardRow);
		}
		else
		{
			int32 NewRowIdx = GrandPrixLeaderboard.GrandPrixRows.AddUnique(NewGrandPrixRow);
			GrandPrixLeaderboard.GrandPrixRows[NewRowIdx].AddRaceRound(LeaderboardRow);
		}
	}

	GrandPrixLeaderboard.GrandPrixRows.StableSort();

	// Recalculate all positions in the leaderboard
	int32 CurrentPosition=1;
	for (int32 i=0; i < GrandPrixLeaderboard.GrandPrixRows.Num(); i++)
	{
		GrandPrixLeaderboard.GrandPrixRows[i].PlayerPosition=CurrentPosition;
		CurrentPosition++;
	}
	
	// Increase round
	GrandPrixLeaderboard.CompletedRoundsCount++;
	
	MARK_PROPERTY_DIRTY_FROM_NAME(AShibRaceManager, GrandPrixLeaderboard, this);

	if (!CurrentGrandPrix) return;
	
	if ((!CurrentGrandPrix->RandomTracks && GrandPrixLeaderboard.CompletedRoundsCount >= CurrentGrandPrix->GrandPrixTracks.Num())
		|| (CurrentGrandPrix->RandomTracks && GrandPrixLeaderboard.CompletedRoundsCount >= CurrentGrandPrix->RaceRounds))
	{
		// TODO: Send the data to the Tournament API if the GP is over
		// Make sure to verify that the ShibUserId is valid for each entry
		// An invalid ShibUserId means that this row is an AI and we don't want to send this row to the tournament API
	}
}

int32 AShibRaceManager::GetTotalGrandPrixRounds() const
{
	if (CurrentGrandPrix)
	{
		if (CurrentGrandPrix->RandomTracks)
		{
			return CurrentGrandPrix->RaceRounds;
		}
		
		return CurrentGrandPrix->GrandPrixTracks.Num();
		
	}
	return INDEX_NONE;
}

void AShibRaceManager::SetExpectedNumbOfPlayers(const int32 NumbOfPlayers)
{
	if (!HasAuthority()) return;

	ExpectedNumbOfPlayers = NumbOfPlayers;
}

void AShibRaceManager::OnRep_CurrentGrandPrix()
{
}
