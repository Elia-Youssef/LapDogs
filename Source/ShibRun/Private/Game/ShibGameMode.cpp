// Copyright Shiba Inu Games LLC.

#include "Game/ShibGameMode.h"
#include "OnlineSubsystemUtils.h"
#include "Actors/ShibRaceManager.h"
#include "Utils/ShibTypes.h"
#include "Kismet/KismetMathLibrary.h"
#include "Game/ShibGameState.h"
#include "Kismet/GameplayStatics.h"
#include "LapDogs/LapDogsApisSubsystem.h"
#include "Player/ShibAiController.h"
#include "Player/ShibBaseController.h"
#include "Player/ShibController.h"
#include "Player/ShibPlayerState.h"

void AShibGameMode::StartPlay()
{
	Super::StartPlay();

	// If all players are ready, we start the countdown right away
	if (GetMatchState() == MatchState::WaitingToStart && IsAllPlayersReadyToRace())
	{
		SetMatchState(MatchState::Countdown);
	}
}

void AShibGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	
	if (MatchState == MatchState::WaitingToStart)
	{
		if (IsAllPlayersReadyToRace())
		{
			// if all players we're already ready to start, we can start the countdown now
			SetMatchState(MatchState::Countdown);	
		}
	}
}

void AShibGameMode::PreLogout(APlayerController* InPlayerController)
{
	if (ConnectedControllers.Contains(InPlayerController))
	{
		if (auto* ShibPS = InPlayerController->GetPlayerState<AShibPlayerState>())
		{
			if (ShibPS && ShibPS->GetPlayerIsReadyToRace())
			{
				NumPlayersInitializedAndReady--;
			}
		
			RemovePlayerStartOwner(Cast<AShibBasePlayerState>(ShibPS));
		}
	}
	
	Super::PreLogout(InPlayerController);
	
	UE_LOG(LogShibGameMode, Log, TEXT("Player is leaving: %d players connected, %d players initialized and ready"),ConnectedControllers.Num(), NumPlayersInitializedAndReady);
}

void AShibGameMode::BeginPlay()
{
	Super::BeginPlay();

	// We always want to have a valid race manager actor present in the level
	// Otherwise the game cannot work
	check(RaceManager);
}

void AShibGameMode::HandleMatchCountdown()
{
	//Make sure the session is full before starting the countdown phase
	FillSessionWithAIs();
	
	// Disable movement for now until the race starts
	EnableShibCharacters(false, true);
	
	// Start updating race leaderboard in the Game State
	if (AShibGameState* ShibGS = GetGameState<AShibGameState>()) ShibGS->StartLeaderboardUpdate();
	
	MatchTimeCounter = CountdownMatchTime; // Set the main timer value to pre match time
	
	// Start Countdown timer
	FTimerDelegate CountdownTimerDelegate;
	CountdownTimerDelegate.BindUObject(this, &ThisClass::MatchTimer);
	GetWorldTimerManager().SetTimer(CountdownTimerHandle, CountdownTimerDelegate, 1.f, true, 1.f);
}

void AShibGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	MatchTimeCounter = WaitingForPlayersMatchTime; // Set the main timer value to waiting for players time
	
	// Start waiting for players timer
	FTimerDelegate WaitingForPlayersTimerDelegate;
	WaitingForPlayersTimerDelegate.BindUObject(this, &ThisClass::MatchTimer);
	GetWorldTimerManager().SetTimer(WaitingForPLayersTimerHandle, WaitingForPlayersTimerDelegate, 1.f, true, 1.f);
}

void AShibGameMode::HandleMatchHasStarted()
{
	//Super::HandleMatchHasStarted();
	MatchStartTime = UKismetMathLibrary::UtcNow();
	
	// Enable character movement for everybody when race starts
	EnableShibCharacters(true, false);
	
	MatchTimeCounter = MatchTime; // Set the main timer value to match time
	// Start match timer
	FTimerDelegate MatchTimerDelegate;
	MatchTimerDelegate.BindUObject(this, &ThisClass::MatchTimer);
	GetWorldTimerManager().SetTimer(MatchTimerHandle, MatchTimerDelegate, 1.f, true, 1.f);
}

void AShibGameMode::HandleMatchEndPending()
{
	MatchTimeCounter = PostMatchTime; // Set the main timer value to match time
	// Start end pending timer
	FTimerDelegate EndPendingTimerDelegate;
	EndPendingTimerDelegate.BindUObject(this, &ThisClass::MatchTimer);
	GetWorldTimerManager().SetTimer(EndPendingTimerHandle, EndPendingTimerDelegate, 1.f, true, 1.f);
}

void AShibGameMode::HandleMatchHasEnded()
{
	// Disable movement completely
	EnableShibCharacters(false, false);
	
	if (auto ShibGS = GetGameState<AShibGameState>())
	{
		// Stop updating race leaderboard in the Game State
		ShibGS->StopLeaderboardUpdate();
		
		// Validate and generate final leaderboard since we know at this point the race is over
		// We want to do this after we've stopped updating the leaderboard.
		// Then store the race leaderboard in the race manager actor
		TArray<FLeaderboardRow> ResultLeaderboard;
		if (RaceManager && ShibGS->ValidateAndGenerateFinalLeaderboard(ResultLeaderboard))
			RaceManager->AddGrandPrixRound(ResultLeaderboard);

		// Send player stats through Shib APIs
		if (auto GI = GetShibGI(); GI->bUseShibApis)
		{
			auto OSS = Online::GetSubsystem(this->GetWorld());
			auto SessionInterface = OSS->GetSessionInterface();
			auto CurrentSession = SessionInterface->GetNamedSession(ShibGameSession);
			auto LdAPIs = GetGameInstance()->GetSubsystem<ULapDogsApisSubsystem>();
			
			if (CurrentSession && LdAPIs)
			{
				FSendGameStatsRequest Request;
				
				Request.EosSession = CurrentSession->GetSessionIdStr();
				Request.MapTitle = UGameplayStatics::GetCurrentLevelName(this->GetWorld());
				Request.LapNumber = ShibGS->RaceLaps; // Temp this could change in the future
				
				for (auto PlayerResult : ResultLeaderboard)
				{
					if (PlayerResult.bIsABot) continue;

					FPlayerStatsRequest NewPlayerData;
					NewPlayerData.UserId = PlayerResult.ShibUserId;
					NewPlayerData.Position = PlayerResult.PlayerPosition;
					NewPlayerData.XP = 0; // TODO: We currently don't have a leveling system in the game
					NewPlayerData.Deaths = PlayerResult.Deaths;
					NewPlayerData.Time = PlayerResult.RaceTimeInSeconds;
					NewPlayerData.DogClass = int8(PlayerResult.DogClass);
					NewPlayerData.distanceRunning = FMath::FloorToInt(PlayerResult.DistanceRan);
					NewPlayerData.raceFinishes = PlayerResult.bRaceFinished;
					NewPlayerData.TumbledOtherShibs = PlayerResult.TumbledShibs;
					NewPlayerData.PickupItemsUsed = PlayerResult.PickupItemsUsed;
					
					Request.Players.Add(NewPlayerData);
				}

				// We cached the request here to keep it in memory if anything goes wrong
				CachedRaceResultsRequest = Request;
				
				UE_LOG(LogShibGameMode, Log, TEXT("Sending race stats through the Shib APIs"));
				LdAPIs->OnSendGameStatsDelegate.AddDynamic(this, &AShibGameMode::HandleSendGameStatsComplete);
				LdAPIs->SendGameStats(Request);
			}
		}
	}
	
	MatchTimeCounter = EndMatchTime; // Set the main timer value to end match time
	// Start end match timer
	FTimerDelegate EndTimerDelegate;
	EndTimerDelegate.BindUObject(this, &ThisClass::MatchTimer);
	GetWorldTimerManager().SetTimer(EndTimerHandle, EndTimerDelegate, 1.f, true, 1.f);
}

void AShibGameMode::ClearGameTimers()
{
	GetWorldTimerManager().ClearTimer(WaitingForPLayersTimerHandle);
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	GetWorldTimerManager().ClearTimer(MatchTimerHandle);
	GetWorldTimerManager().ClearTimer(EndPendingTimerHandle);
	GetWorldTimerManager().ClearTimer(EndTimerHandle);
	
	GetWorldTimerManager().ClearTimer(SpectatorCamerasHandle);
}

void AShibGameMode::NotifyCurrentMatchState()
{
	// Notify players of the new match state and how much time it will remain in this state (time is only for certain types of match state)
	float MatchStateTime=0.f;

	if (MatchState == MatchState::Countdown)
	{
		MatchStateTime=CountdownMatchTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		MatchStateTime=MatchTime;
	}
	else if (MatchState == MatchState::EndPending)
	{
		MatchStateTime=PostMatchTime;
	}
	else if (MatchState == MatchState::WaitingPostMatch)
	{
		MatchStateTime=EndMatchTime;
	}
	
	for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
	{
		if (AShibController* PC = Cast<AShibController>(Iterator->Get()))
		{
			PC->Client_OnMatchStateChangeNotify(MatchState, MatchStateTime);
		}
	}
}

bool AShibGameMode::IsAllPlayersReadyToRace()
{
	// If the number of players initialized and ready is equal (or more) to the number count from the base class,
	// it means every player in this session possess a pawn and everyone is ready to race
	// Spawned players vs Connected players comparison
	if (RaceManager && NumPlayersInitializedAndReady >= RaceManager->GetExpectedNumbOfPlayers())
	{
		UE_LOG(LogShibGameMode, Log, TEXT("%d/%d connected players are ready. Race can start..."), NumPlayersInitializedAndReady, RaceManager->GetExpectedNumbOfPlayers());
		return true;
	}
	
	UE_LOG(LogShibGameMode, Log, TEXT("%d/%d connected players are ready. Waiting for all players to be ready..."), NumPlayersInitializedAndReady, RaceManager->GetExpectedNumbOfPlayers());
	return false;
}

void AShibGameMode::FillSessionWithAIs()
{
	if(!RaceManager || !RaceManager->bSpawnAI) return;
	
	int32 EmptySlots=0;
	if (const auto GI = GetShibGI())
	{
		EmptySlots = GI->DefaultNumConnections - GetNumPlayers();
	}

	// Restart all previously created Ai controllers
	for (auto AiController : RaceManager->AIControllers)
	{
		if (EmptySlots > 0)
		{
			RestartPlayer(AiController);
			EmptySlots--;
		}
		else // If we have enough players, any leftover AI controllers get destroyed. This should never happen, but it's here just in case
		{
			RaceManager->AIControllers.Remove(AiController);
			AiController->Destroy();
		}
	}
	
	if (EmptySlots==0) return;
	// If we still have empty slot, we spawn new AI pawns
	for(int i = 1; i <= EmptySlots; ++i)
	{
		RestartPlayer(nullptr); // Passing a null controller here will create a new AI controller with his pawn
	}
}

void AShibGameMode::HandleSendGameStatsComplete(bool bSuccessful)
{
	if(auto LdAPIs = GetGameInstance()->GetSubsystem<ULapDogsApisSubsystem>())
		LdAPIs->OnSendGameStatsDelegate.RemoveAll(this);
	
	if (bSuccessful)
	{
		UE_LOG(LogShibGameMode, Log, TEXT("Race stats successfully sent to the Shib APIs"));
	}
	else
	{
		UE_LOG(LogShibGameMode, Error, TEXT("Race stats were not successfully sent to Shib APIs. Something went wrong when submiting the request to the Shib APIs."));
	}
}

void AShibGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (MatchState == MatchState::Countdown || MatchState == MatchState::InProgress || MatchState == MatchState::EndPending || MatchState == MatchState::WaitingPostMatch)
	{
		KickPlayer(NewPlayer, FText::FromString("The race has started, no new participants are allowed to join."));
		return;
	}
	
	// Spawn his character right away when joining
	RestartPlayer(NewPlayer);
	
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

void AShibGameMode::MatchTimer()
{
	MatchTimeCounter--;

	// Do something based on the current match state when the timer reaches zero
	if (MatchTimeCounter == 0)
	{
		if (MatchState == MatchState::WaitingToStart)
		{
			// If were here it means not all players we're able to join the race map and get ready
			// Kick out everyone that isn't ready and start race countdown
			for (auto* PC : ConnectedControllers)
			{
				if (auto* PS = PC->GetPlayerState<AShibPlayerState>(); !PS->GetPlayerIsReadyToRace())
				{
					KickPlayer(PC, FText::FromString("The player was unable to get ready in time due to slow initialization."));
				}
			}
			
			SetMatchState(MatchState::Countdown);	
		}
		else if (MatchState == MatchState::Countdown)
		{
			SetMatchState(MatchState::InProgress);	
		}
		else if (MatchState == MatchState::InProgress || MatchState == MatchState::EndPending) // Here the race is over for both match state when timer reaches zero
		{
			SetMatchState(MatchState::WaitingPostMatch);
		}
		else if (MatchState == MatchState::WaitingPostMatch)
		{
			// Final clean up and move to the next level
			ClearGameTimers();
			TravelToNextLevel();
		}
	}
}

void AShibGameMode::OnMatchStateSet()
{
	FGameModeEvents::OnGameModeMatchStateSetEvent().Broadcast(MatchState);

	// Always clear game timers since match state changes
	// We'll probably start a new timer depending on the new match state after
	ClearGameTimers();
	
	//Notify players
	NotifyCurrentMatchState();
	
	// Call change callbacks
	if (MatchState == MatchState::WaitingToStart)
	{
		HandleMatchIsWaitingToStart();
	}
	else if (MatchState == MatchState::Countdown)
	{
		HandleMatchCountdown();
	}
	else if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::EndPending)
	{
		HandleMatchEndPending();
	}
	else if (MatchState == MatchState::WaitingPostMatch)
	{
		HandleMatchHasEnded();
	}
	else if (MatchState == MatchState::LeavingMap)
	{
		HandleLeavingMap();
	}
	else if (MatchState == MatchState::Aborted)
	{
		HandleMatchAborted();
	}
}

void AShibGameMode::PlayerIsReadyToRace()
{
	if (HasMatchStarted()) return;

	NumPlayersInitializedAndReady++;
	
	// Start the race countdown if all the players joined, are spawned and ready
	// Important to check for the total numb of player here because maybe some players are still travelling to the current map
	if (IsAllPlayersReadyToRace() && GetMatchState() == MatchState::WaitingToStart )
	{
		SetMatchState(MatchState::Countdown);
	}
}

void AShibGameMode::PlayerFinishedRace(AController* Controller, FLeaderboardRow& PlayerStats)
{
	if (AShibController* ShibPC = Cast<AShibController>(Controller))
	{
		ShibPC->Client_OnMatchStateChangeNotify(MatchState::PlayerFinishedRace);
	} else if (AShibAiController* ShibAiController = Cast<AShibAiController>(Controller))
	{
		ShibAiController->StopAIBrain();
	}
	
	// If it's the first player to cross the finish line and the race is still in progress,
	// We change the match state to end pending to notify players that the race will end soon.
	if (MatchState == MatchState::InProgress)
	{
		SetMatchState(MatchState::EndPending);
	}
	else // If it's the last player crossing the finish line, start the final state before moving to the next map
	{
		if (const auto GI = GetShibGI())
		{
			if (PlayerStats.PlayerPosition >= GI->DefaultNumConnections) SetMatchState(MatchState::WaitingPostMatch);
		}
	}
}
