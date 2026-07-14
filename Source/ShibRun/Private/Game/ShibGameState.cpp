// Copyright Shiba Inu Games LLC.

#include "Game/ShibGameState.h"
#include "Actors/RouteSpline.h"
#include "Character/ShibCharacter.h"
#include "Components/SplineComponent.h"
#include "Game/ShibGameMode.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Player/ShibController.h"
#include "Player/ShibPlayerState.h"
#include "Utils/ShibTypes.h"
#include "AbilitySystem/ShibAbilitySystemComponent.h"
#include "GameplayTags/ShibGameplayTags.h"

void AShibGameState::BeginPlay()
{
	Super::BeginPlay();
	
	// Make sure we start fresh with empty leaderboards
	RaceLeaderboard.Empty();
	FinalLeaderboard.Empty();
	
	if (UShibGameInstance* GI = GetGameInstance<UShibGameInstance>())
	{
		RaceLeaderboard.Reserve(GI->DefaultNumConnections);
		FinalLeaderboard.Reserve(GI->DefaultNumConnections);
	}
	
	// Find the track spline in the level, return if we can't find it
	TrackSpline = Cast<ARouteSpline>(UGameplayStatics::GetActorOfClass(this, ARouteSpline::StaticClass()));
	// Get the lenght of the track, this value will be use multiple times in this class
	if (TrackSpline) TrackSplineLength = TrackSpline->Spline->GetSplineLength();
}

void AShibGameState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	OnRaceLeaderboardUpdate.RemoveAll(this);
	OnFinalLeaderboardUpdate.RemoveAll(this);
	OnRaceLapFinish.RemoveAll(this);
	
	Super::EndPlay(EndPlayReason);
}

void AShibGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS(AShibGameState, RaceLeaderboard, Params);
	DOREPLIFETIME_WITH_PARAMS(AShibGameState, FinalLeaderboard, Params);
	DOREPLIFETIME(AShibGameState, RaceManager);
}

bool AShibGameState::GetLeaderboardRowByPlayerNetId(FUniqueNetIdRepl PlayerId, FLeaderboardRow& LeaderboardRowFound, const bool bFinal)
{
	if (!PlayerId.GetUniqueNetId().IsValid()) return false;

	for (auto Row : bFinal? FinalLeaderboard : RaceLeaderboard)
	{
		if (!Row.PlayerId.GetUniqueNetId().IsValid()) return false;

		const FUniqueNetId& CurrentRowPlayerId = *Row.PlayerId.GetUniqueNetId().Get();
		const FUniqueNetId& InPlayerUniqueToCompareTo = *PlayerId.GetUniqueNetId().Get();

		if (CurrentRowPlayerId == InPlayerUniqueToCompareTo)
		{
			LeaderboardRowFound = Row;
			return true;
		}
	}
	return false;
}

int32 AShibGameState::GetLeaderboardRowIndexByPlayerNetId(FUniqueNetIdRepl PlayerId, const bool bFinal)
{
	if (!PlayerId.GetUniqueNetId().IsValid()) return false;

	TArray<FLeaderboardRow> Leaderboard = bFinal? FinalLeaderboard : RaceLeaderboard;

	for (int32 i=0; i<Leaderboard.Num(); i++)
	{
		if (!Leaderboard[i].PlayerId.GetUniqueNetId().IsValid()) return false;

		const FUniqueNetId& CurrentRowPlayerId = *Leaderboard[i].PlayerId.GetUniqueNetId().Get();
		const FUniqueNetId& InPlayerUniqueToCompareTo = *PlayerId.GetUniqueNetId().Get();

		if (CurrentRowPlayerId == InPlayerUniqueToCompareTo)
		{
			return i;
		}
	}
	
	return INDEX_NONE;
}

void AShibGameState::PlayerCrossedFinishLine(APlayerState* PlayerStateRef, FDateTime FinishTime)
{
	if (!HasAuthority()) return;

	UpdateLeaderboard();

	// Safety check to make sure we always have a valid reference to the track spline
	if (!TrackSpline)
	{
		// Find the track spline in the level, return if we can't find it
		if(TrackSpline = Cast<ARouteSpline>(UGameplayStatics::GetActorOfClass(this, ARouteSpline::StaticClass())); !TrackSpline) return;
	}
	
	AShibGameMode* RaceShibGM = Cast<AShibGameMode>(GetShibGM());
	AShibPlayerState* RaceShibPS = Cast<AShibPlayerState>(PlayerStateRef);
	
	if (!RaceShibGM || !RaceShibPS) return;

	// Record lap time
	if (int32 RowIndex = GetLeaderboardRowIndexByPlayerNetId(RaceShibPS->GetUniqueId()); RowIndex != INDEX_NONE)
	{
		FTimespan RaceLapTime = FinishTime - RaceShibGM->GetMatchStartTime();
		if (RaceShibPS->GetCurrentLap() == 1) // for the first completed lap
		{
			// This is the time between race start and the player crossing the finish line for the first time
			float InitialLapStartTime = RaceLeaderboard[RowIndex].LapsTime.Pop();
			RaceLeaderboard[RowIndex].LapsTime.Add(RaceLapTime.GetTotalSeconds() - InitialLapStartTime);
		}
		else
		{
			RaceLeaderboard[RowIndex].LapsTime.Add(RaceLapTime.GetTotalSeconds());
		}
	}
	
	// When the player cross the finish line for the first time at the start of the race
	// We make sure he crossed it in the correct direction
	if (RaceShibPS->GetCurrentLap() == 0 && RaceShibPS->ConfirmedLapTravelledDistance / TrackSplineLength > 0.8f)
	{
		// Reset confirmed travelled distance because we crossed the finish line
		RaceShibPS->ConfirmedLapTravelledDistance = 1.f; 
		// Increase lap counter to the next lap
		RaceShibPS->IncreaseLapCounter();
		return;
	}
	
	// Confirm that the player has actually completed the entire circuit.
	// Compare the confirmed travelled distance with the total spline length. The result should be more than 80% of completion to be safe
	if (RaceShibPS->ConfirmedLapTravelledDistance / TrackSplineLength < 0.8f) return;
	
	// Reset confirmed travelled distance because we crossed the finish line
	RaceShibPS->ConfirmedLapTravelledDistance = 1.f;
	
	if (RaceShibPS->GetCurrentLap() == RaceLaps)
	{
		RaceShibPS->IncreaseLapCounter();
	
        UpdateLeaderboard();
	
		// Save race stats and add it to the leaderboard
		FLeaderboardRow NewLeaderboardRow;

		if (!GetLeaderboardRowByPlayerNetId(PlayerStateRef->GetUniqueId(), NewLeaderboardRow, false)) return;

		// If the player is already in the leaderboard, we ignore him.
		if (FinalLeaderboard.Find(NewLeaderboardRow) != INDEX_NONE) return;
		
		NewLeaderboardRow.PlayerPosition = FinalLeaderboard.Num()+1;
		NewLeaderboardRow.DistanceRan = TrackSplineLength*RaceLaps;
		NewLeaderboardRow.CurrentLap = RaceLaps;
		NewLeaderboardRow.Points = RaceShibGM->FirstPlayerAwardedPoints - (NewLeaderboardRow.PlayerPosition - 1);
		NewLeaderboardRow.RaceTimeInSeconds = (FinishTime - RaceShibGM->GetMatchStartTime()).GetTotalSeconds();
		NewLeaderboardRow.bRaceFinished = true;
		
		// Add the player to the leaderboard
		FinalLeaderboard.AddUnique(NewLeaderboardRow);
		MARK_PROPERTY_DIRTY_FROM_NAME(AShibGameState, FinalLeaderboard, this);
		
		RaceShibGM->PlayerFinishedRace(PlayerStateRef->GetOwningController(),NewLeaderboardRow);
		// Call the OnRep function directly from the server here to update the host's UI (Listen Server Only)
		// Because OnRep function doesn't fire on the server
		OnRep_FinalLeaderboard();
		return;
	}
	
	// Increase lap counter to the next lap if the player hasn't finished the race
	RaceShibPS->IncreaseLapCounter();

	// Notify when a new lap has started
	if (RaceShibPS->GetCurrentLap() > CurrentRaceLap)
	{
		CurrentRaceLap = RaceShibPS->GetCurrentLap();
		OnRaceLapFinish.Broadcast(CurrentRaceLap);
	}
}

void AShibGameState::StartLeaderboardUpdate()
{
	if (!HasAuthority()) return;

	// Safety check to make sure only one timer is running
	if (LeaderboardUpdateHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(LeaderboardUpdateHandle);
	}

	// Start the timer to update the leaderboard when the race start
	// Find the track spline in the level to start tracking down the player positions
	TrackSpline = Cast<ARouteSpline>(UGameplayStatics::GetActorOfClass(this, ARouteSpline::StaticClass()));
	// If there is no spline, we can't get the current player positions
	if (!ensureMsgf(TrackSpline,
		TEXT("Failed to find route spline actor in the current level. Game State won't be able to track players' positions."))) return;
	
	// Get pawn start position for all connected Player States. This is to know the initial position that will be used in the first leaderboard update. 
	
	for (TObjectPtr<APlayerState> PS : PlayerArray)
	{
		// Make sure we're using the correct player state class
		if (AShibPlayerState* RacePS = Cast<AShibPlayerState>(PS))
		{
			AShibCharacter* ShibChar = Cast<AShibCharacter>(RacePS->GetPawn());
			if(!ShibChar) continue;
			
			// Here we calculate the player position based on the travelled distance.
			const FVector ClosestLocation = TrackSpline->Spline->FindLocationClosestToWorldLocation(ShibChar->GetActorLocation(), ESplineCoordinateSpace::World);
			const float DistanceFromStart = TrackSpline->Spline->GetDistanceAlongSplineAtLocation(ClosestLocation, ESplineCoordinateSpace::World);
			RacePS->ConfirmedLapTravelledDistance = DistanceFromStart;
			RacePS->CurrentLap = 0;
			
			if (ShibChar->IsLocallyControlled()) // to update current lap on Host's UI (Listen server only)
			{
				RacePS->OnRep_CurrentLap();
			}

			if (IsValid(ShibChar->Ability))
			{
				ShibChar->Ability.Get()->AbilityStartedDelegate.AddDynamic(this, &ThisClass::RecordAbilityInLeaderboard);
			}
		}
	}
	
	FTimerDelegate LiveLeaderboardDelegate;
	LiveLeaderboardDelegate.BindUObject(this, &ThisClass::UpdateLeaderboard);
	GetWorldTimerManager().SetTimer(LeaderboardUpdateHandle, LiveLeaderboardDelegate, LeaderboardUpdateFrequency, true);
}

void AShibGameState::StopLeaderboardUpdate()
{
	if (LeaderboardUpdateHandle.IsValid())
	{
		GetWorldTimerManager().ClearTimer(LeaderboardUpdateHandle);
	}

	for (TObjectPtr<APlayerState> PS : PlayerArray)
	{
		AShibCharacter* ShibChar = Cast<AShibCharacter>(PS->GetPawn());
		if(!ShibChar) continue;

		if (IsValid(ShibChar->Ability))
		{
			ShibChar->Ability.Get()->AbilityStartedDelegate.RemoveDynamic(this, &ThisClass::RecordAbilityInLeaderboard);
		}
	}

}

bool AShibGameState::ValidateAndGenerateFinalLeaderboard(TArray<FLeaderboardRow>& FinalRaceLeaderboard)
{
	// AShibPlayerState* RacePS;
	AShibGameMode* RaceShibGM = Cast<AShibGameMode>(GetShibGM());
	
	if (!RaceShibGM) return false;

	// Snapshot of time when the race finished
	// Race start time - current time
	FTimespan FinishTime = UKismetMathLibrary::UtcNow() - RaceShibGM->GetMatchStartTime();
	bool LeaderboardIsDirty = false; // Flag to know if we modified the leaderboard in this function

	// One final update of the players positions before generating the full race leaderboard
	// At this point, players can't move anymore
	UpdateLeaderboard();
	
	// Check if all race leadeboard rows are part of the final leaderboard
	// If one or more are missing, it means the players didn't finish the race and we want to add them in the final leaderboard
	for (auto RaceLeaderboardRow : RaceLeaderboard)
	{
		// If we don't find the row, it means the player isn't in the list and we need to add it
		if (FinalLeaderboard.Find(RaceLeaderboardRow) == INDEX_NONE)
		{
			//Mark the leaderboard dirty since we will modify it
			LeaderboardIsDirty = true;
			
			// Add the missing player in the leaderboard
			RaceLeaderboardRow.DistanceRan = RaceLeaderboardRow.CurrentLap != 0 ? RaceLeaderboardRow.DistanceRan : 0.f;
			RaceLeaderboardRow.RaceTimeInSeconds = 0.f;
			RaceLeaderboardRow.Points = 0;
			FinalLeaderboard.AddUnique(RaceLeaderboardRow);
		}
	}

	// If we didn't add any new row in the leaderboard, we can skip the rest of this function
	if (!LeaderboardIsDirty)
	{
		FinalRaceLeaderboard = FinalLeaderboard;
		return true;
	}
	
	// We can sort this array without a binary predicate because the < operator is defined in the struct class
	// Here we use stable sort because the order is very important
	FinalLeaderboard.StableSort();

	// Give points to the players and validate their positions again before doing so
	for (int i=0; i < FinalLeaderboard.Num(); i++)
	{
		if (FinalLeaderboard[i].RaceTimeInSeconds == 0.f) // if the race time is zero, we know this player wasn't able to finish the race
		{
			// Indicate that this player didn't finish the race
			FinalLeaderboard[i].bRaceFinished = false;

			// Set finish time
			FinalLeaderboard[i].RaceTimeInSeconds = FinishTime.GetTotalSeconds();
			
			// Make sure the position of the player is the right one
			// We don't only count on the current player position updated during the race, we want to validate it using the leaderboard itself too
			FinalLeaderboard[i].PlayerPosition = i+1;
			
			// Give the player his points
			FinalLeaderboard[i].Points = RaceShibGM->FirstPlayerAwardedPoints - (FinalLeaderboard[i].PlayerPosition - 1);
		}
	}
	
	MARK_PROPERTY_DIRTY_FROM_NAME(AShibGameState, FinalLeaderboard, this);
	// Call the OnRep function directly from the server here to update the host's UI (Listen Server Only)
	// Because OnRep function doesn't fire on the server
	OnRep_FinalLeaderboard();

	FinalRaceLeaderboard = FinalLeaderboard;
	return true;
}

void AShibGameState::ValidateRaceLeaderboard()
{
	bool LeaderboardIsDirty = false;
	for (auto LeaderboardRow : RaceLeaderboard)
	{
		APlayerState* PS = GetPlayerStateFromUniqueNetId(LeaderboardRow.PlayerId);

		if (!PS)
		{
			RaceLeaderboard.Remove(LeaderboardRow);
			LeaderboardIsDirty = true;
		}
		else if (!PS->GetPawn())
		{
			RaceLeaderboard.Remove(LeaderboardRow);
			LeaderboardIsDirty = true;
		}
	}

	if (LeaderboardIsDirty)
	{
		UpdateLeaderboard();
	}
}

void AShibGameState::OnRep_MatchState()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, FString("Match State Changed to: ") + MatchState.ToString());
	Super::OnRep_MatchState();
}

void AShibGameState::UpdateLeaderboard()
{
	// Safety check to make sure we always have a valid reference to the track spline
	if (!TrackSpline)
	{
		// Find the track spline in the level, return if we can't find it
		if(TrackSpline = Cast<ARouteSpline>(UGameplayStatics::GetActorOfClass(this, ARouteSpline::StaticClass())); !TrackSpline) return;
	}
	
	// Get pawn travelled distance for all connected Player States
	for (TObjectPtr<APlayerState> PS : PlayerArray)
	{
		// Make sure we're using the correct player state class
		if (AShibPlayerState* RacePS = Cast<AShibPlayerState>(PS))
		{
			if (!RacePS->GetPawn()) continue;
			
			// Here we calculate the player position based on the travelled distance.
			const FVector ClosestLocation = TrackSpline->Spline->FindLocationClosestToWorldLocation(RacePS->GetPawn()->GetActorLocation(), ESplineCoordinateSpace::World);
			const float DistanceFromStart = TrackSpline->Spline->GetDistanceAlongSplineAtLocation(ClosestLocation, ESplineCoordinateSpace::World);
			
			const float CompletionPercentage = (DistanceFromStart - RacePS->ConfirmedLapTravelledDistance) / TrackSplineLength;
			
			// Anti cheat check to make sure the player never travel more than 20% of the entire racetrack length in between two updates. This is literally impossible and should never happen.
			// this way, we can confirm that the player has actually completed the entire circuit.
			if ( CompletionPercentage < 0.2f && CompletionPercentage > -0.2f)
			{
				RacePS->ConfirmedLapTravelledDistance = DistanceFromStart;
			}

			//TODO: Alert player that he's going in the wrong direction when completion percentage is less below zero?
			
			// Amount of laps completed (Current lap - 1) * Length of the spline + Current lap travelled distance
			// Using this method, we don't need to sort players using the number of laps AND position
			const float CurrentDistanceRan = (RacePS->GetCurrentLap()-1)*TrackSplineLength + RacePS->ConfirmedLapTravelledDistance;
			
			if (int32 RowIdx = GetLeaderboardRowIndexByPlayerNetId(RacePS->GetUniqueId()); RowIdx!=INDEX_NONE && RaceLeaderboard.IsValidIndex(RowIdx))
			{
				RaceLeaderboard[RowIdx].PlayerPosition = 0; // Reset player position
				RaceLeaderboard[RowIdx].CurrentLap = RacePS->GetCurrentLap();
				RaceLeaderboard[RowIdx].DistanceRan = CurrentDistanceRan;
			}
			else
			{
				FLeaderboardRow LeaderboardRow(RacePS->GetUniqueId(), RacePS->CachedShibUserId, RacePS->GetPlayerName(),
					RacePS->SelectedShibClass, RacePS->IsABot(), RacePS->GetCurrentLap(), CurrentDistanceRan, 0);
		
				RaceLeaderboard.AddUnique(LeaderboardRow);
			}
		}
	}

	// Sort leaderboard, this will use Distance ran to sort the array
	RaceLeaderboard.StableSort();

	// Set player positions with sorted array
	int32 CurrentPosition=1;
	for (int32 i=0; i<RaceLeaderboard.Num(); i++)
	{
		RaceLeaderboard[i].PlayerPosition = CurrentPosition;
		CurrentPosition++;
	}
	
	// Call the OnRep function directly from the server here to update the host's UI (Listen Server Only)
	// Because OnRep function doesn't fire on the server
	MARK_PROPERTY_DIRTY_FROM_NAME(AShibGameState, RaceLeaderboard, this);
	OnRep_RaceLeaderboard();
}

void AShibGameState::RecordAbilityInLeaderboard(const FGameplayTag AbilityTag, const AActor* AbilityOwner, const AActor* AbilityInstigator,
	float ExecutionTime)
{
	if (!AbilityOwner || !AbilityInstigator || !AbilityTag.IsValid() || RaceLeaderboard.IsEmpty()) return;

	const AShibCharacter* ShibOwner = Cast<AShibCharacter>(AbilityOwner);
	const AShibCharacter* ShibInstigator = Cast<AShibCharacter>(AbilityInstigator);
	if (!ShibOwner || !ShibInstigator) return;
	
	AShibPlayerState* PsOwner = Cast<AShibPlayerState>(ShibOwner->GetPlayerState());
	AShibPlayerState* PsInstigator = Cast<AShibPlayerState>(ShibInstigator->GetPlayerState());
	if (!PsOwner || !PsInstigator) return;
	
	int32 RowIndex;
	if (AbilityTag.MatchesTagExact(TAG_Effect_Movement_Tumble)) // Tumble
	{
		if (RowIndex = GetLeaderboardRowIndexByPlayerNetId(PsInstigator->GetUniqueId()); RowIndex != INDEX_NONE)
		{
			RaceLeaderboard[RowIndex].TumbledShibs += 1;
		}
	}
	else if (AbilityTag.MatchesTagExact(TAG_Status_Death_Dead)) // Death
	{
		if (RowIndex = GetLeaderboardRowIndexByPlayerNetId(PsOwner->GetUniqueId()); RowIndex != INDEX_NONE)
		{
			RaceLeaderboard[RowIndex].Deaths += 1;
		}
	}
	else if (AbilityTag.MatchesTag(TAG_PickupAbility)) // Any Pickups
	{
		if (RowIndex = GetLeaderboardRowIndexByPlayerNetId(PsOwner->GetUniqueId()); RowIndex != INDEX_NONE)
		{
			RaceLeaderboard[RowIndex].PickupItemsUsed += 1;
		}
	}
}

void AShibGameState::OnRep_RaceLeaderboard()
{
	OnRaceLeaderboardUpdate.Broadcast();
}

void AShibGameState::OnRep_FinalLeaderboard()
{
	OnFinalLeaderboardUpdate.Broadcast();
}
