// Copyright Shiba Inu Games LLC.

#include "Game/ShibLobbyGameMode.h"
#include "DedicatedServerMatchmakingBeaconHost.h"
#include "ShibMatchmakingEOS.h"
#include "Actors/ShibRaceManager.h"
#include "Game/ShibGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShibLobbyPlayerState.h"

class UShibMatchmakingEOS;

void AShibLobbyGameMode::UpdateConnectedCtrlGameRoom()
{
	bool IsDedicatedServer = this->GetWorld()->GetNetMode() == NM_DedicatedServer;
	
	for (const auto* Ctrl : ConnectedControllers)
	{
		if (auto* ShibPS = Cast<AShibLobbyPlayerState>(Ctrl->PlayerState))
		{
			ShibPS->ClientUpdateGameRoom(LobbyTimeCounter, IsDedicatedServer, LobbyStatus);
		}
	}

	// Notify host when a new player join, this will make sure the host don't start the race before everybody is ready
	if (!IsDedicatedServer && LobbyStatus != ELobbyStatus::RaceIsStarting)
	{
		OnRaceReadyToStart.Broadcast(IsAllPlayersReadyToStart());
	}
}

void AShibLobbyGameMode::ForceUpdateGameRoom(AShibLobbyPlayerState* TargetPS)
{
	TargetPS->ClientUpdateGameRoom(LobbyTimeCounter, this->GetWorld()->GetNetMode() == NM_DedicatedServer, LobbyStatus);
}

bool AShibLobbyGameMode::IsAllPlayersReadyToStart()
{
	return ConnectedControllers.Num() != 0 && NumPlayersInitializedAndReady >= RaceManager->GetExpectedNumbOfPlayers() && LobbyStatus == ELobbyStatus::WaitingToStart;
}

void AShibLobbyGameMode::NotifyPlayerIsReadyToStart(AShibLobbyPlayerState* PSReady, bool bReady)
{
	if (PSReady->bIsReady == bReady) return;

	PSReady->bIsReady = bReady;
	
	if (bReady)
		NumPlayersInitializedAndReady++;
	else
		NumPlayersInitializedAndReady--;

	UE_LOG(LogShibGameMode, Log, TEXT("Player is ready to start %hhd: %d players connected, %d expected players, %d players initialized and ready"), bReady, ConnectedControllers.Num(), RaceManager->GetExpectedNumbOfPlayers(), NumPlayersInitializedAndReady);
	
	// Check if all players are ready to start if the race isn't started yet
	if (LobbyStatus != ELobbyStatus::RaceIsStarting)
	{
		RaceReadyToStart(IsAllPlayersReadyToStart());	
	}
}

void AShibLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	// We always want to have a valid race manager actor present in the level
	// Otherwise the game cannot work
	check(RaceManager);

	// On dedicated server
	// The Matchmaking beacon will fire an event when all players joined, we don't need to count on the amount of connected players and expected players to start the race
	if (ADedicatedServerMatchmakingBeaconHost* BeaconFound = Cast<ADedicatedServerMatchmakingBeaconHost>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADedicatedServerMatchmakingBeaconHost::StaticClass())); BeaconFound && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		BeaconFound->OnMatchReadyToStart.AddDynamic(this, &AShibLobbyGameMode::HandleOnMatchReadyToStart);
		LobbyTimeCounter = BeaconFound->ReservationTimeoutSeconds;
	}
	else 
	{
		LobbyTimeCounter = LobbyTime;
	}
	
	// On listen server, we get the expected amount of players from the matchmaking system event "OnComplet" since we don't have a beacon this time get notified when all players joined the session.
	if (auto* MatchmakingSubsystem = GetShibGI()->GetSubsystem<UShibMatchmakingEOS>())
	{
		MatchmakingSubsystem->OnComplete.AddDynamic(this, &AShibLobbyGameMode::HandleMatchmakingCompleted);
	}
}

void AShibLobbyGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	// Start lobby timer when the first player enter the lobby room
	if (!LobbyTimerHandle.IsValid())
	{
		// Start Lobby timer
		FTimerDelegate LobbyTimerDelegate;
		LobbyTimerDelegate.BindUObject(this, &ThisClass::LobbyTimer);
		GetWorldTimerManager().SetTimer(LobbyTimerHandle, LobbyTimerDelegate, 1.f, true, 0.f);
	}
}

void AShibLobbyGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// On listen server only
	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		if (RaceManager->GetExpectedNumbOfPlayers() > 0 && ConnectedControllers.Num() >= RaceManager->GetExpectedNumbOfPlayers())
		{
			UE_LOG(LogShibGameMode, Log, TEXT("%d players connected. %d players are expected. Waiting to start..."),ConnectedControllers.Num(), RaceManager->GetExpectedNumbOfPlayers());
			LobbyStatus = ELobbyStatus::WaitingToStart;
		}
	}
	
	UpdateConnectedCtrlGameRoom();
}

void AShibLobbyGameMode::Logout(AController* Exiting)
{
	// Check if he was already initialized
	if (auto PS = Cast<AShibLobbyPlayerState>(Exiting->PlayerState))
	{
		if (PS->bIsReady)
			NumPlayersInitializedAndReady--;
	}
	
	Super::Logout(Exiting);

	UE_LOG(LogShibGameMode, Log, TEXT("Player is leaving: %d players connected, %d expected players, %d players initialized and ready"),ConnectedControllers.Num(), RaceManager->GetExpectedNumbOfPlayers(), NumPlayersInitializedAndReady);
	
	UpdateConnectedCtrlGameRoom();
	
	RaceReadyToStart(IsAllPlayersReadyToStart());
}

void AShibLobbyGameMode::RaceReadyToStart(bool bIsReady)
{
	// If the race isn't started yet, we start it and we will come back here after
	if (bIsReady && LobbyStatus != ELobbyStatus::RaceIsStarting)
	{
		StartSession();
		return;
	}
	
	if (bIsReady && !StartingRaceTimerHandle.IsValid())
	{
		if (auto* SessionsSubsystem = GetShibGI()->GetSubsystem<UShibSessionsEOS>())
			SessionsSubsystem->OnStartSessionComplete.RemoveDynamic(this, &AShibLobbyGameMode::RaceReadyToStart);
		
		LobbyTimeCounter = LobbyCountdownTime;

		// Notify the final countdown to players
		UpdateConnectedCtrlGameRoom();

		// Starting final Lobby timer countdown before starting the race
		FTimerDelegate StartingRaceTimerDelegate;
		StartingRaceTimerDelegate.BindUObject(this, &ThisClass::LobbyTimer);
		GetWorldTimerManager().SetTimer(StartingRaceTimerHandle, StartingRaceTimerDelegate, 1.f, true, 1.f);
	}
}

void AShibLobbyGameMode::StartSession()
{
	if (LobbyStatus != ELobbyStatus::RaceIsStarting)
	{
		ClearGameTimers();
		
		UE_LOG(LogShibGameMode, Log, TEXT("%d players connected. %d players were expected. Race is starting..."),ConnectedControllers.Num(), RaceManager->GetExpectedNumbOfPlayers());
		LobbyStatus = ELobbyStatus::RaceIsStarting;
		UpdateConnectedCtrlGameRoom();

		// demo
		RaceReadyToStart(true);
		return;

		// Only start the session if we're not on a dedicated server
		// When on dedicated server, the matchmaking beacon takes care of starting the session for us
		// if (this->GetWorld()->GetNetMode() != NM_DedicatedServer)
		// {
		// 	if (auto* SessionsSubsystem = GetShibGI()->GetSubsystem<UShibSessionsEOS>())
		// 	{
		// 		SessionsSubsystem->OnStartSessionComplete.AddDynamic(this, &AShibLobbyGameMode::RaceReadyToStart);	
		// 		SessionsSubsystem->StartSession();
		// 	}
		// }
		// else
		// {
		// 	RaceReadyToStart(true);
		// }
	}
}

void AShibLobbyGameMode::ClearGameTimers()
{
	GetWorldTimerManager().ClearTimer(LobbyTimerHandle);
	GetWorldTimerManager().ClearTimer(StartingRaceTimerHandle);
}

void AShibLobbyGameMode::LobbyTimer()
{
	LobbyTimeCounter--;

	// Start race when the timer reaches zero
	if (LobbyTimeCounter == 0.f)
	{
		ClearGameTimers();
		
		if (LobbyStatus == ELobbyStatus::RaceIsStarting)
		{
			TravelToNextLevel();
		}
		else
		{
			StartSession();
		}
	}
}

void AShibLobbyGameMode::HandleMatchmakingCompleted(const FString& TeamResults, const FShibMatchmakerHostConfiguration& Request)
{
	if (auto* MatchmakingSubsystem = GetShibGI()->GetSubsystem<UShibMatchmakingEOS>())
	{
		MatchmakingSubsystem->OnComplete.RemoveAll(this);
	}
	
	if (RaceManager)
	{
		// TODO: At some point we will need to extract the Id from the queue name string
		// We first need to define a normalized queue name format
		FString SessionGrandPrixId = Request.QueueName; 
		RaceManager->SetCurrentGrandPrixAsset(SessionGrandPrixId);
	}

	if (GetWorld()->GetNetMode() != NM_DedicatedServer)
	{
		int32 TempNumbOfPlayers=0;
		TArray<FString> OutResult;
		TeamResults.ParseIntoArray(OutResult,TEXT("\n"),true);
	
		for (auto Slot : OutResult)
		{
			if (Slot.Contains("Slot "))
			{
				// e.g. string format: "Slot %d: (AI)"
				FString SlotNumb, PlayerId;
				Slot.Split(TEXT(": "),&SlotNumb,&PlayerId);

				if (PlayerId != "(AI)" && PlayerId != "(none)")
				{
					TempNumbOfPlayers++;
				}
			}
		}

		if (RaceManager) RaceManager->SetExpectedNumbOfPlayers(TempNumbOfPlayers);

		UE_LOG(LogShibGameMode, Log, TEXT("Matchmaking completed: Expected number of players: %d"), TempNumbOfPlayers);
	
		if (ConnectedControllers.Num() == TempNumbOfPlayers)
		{
			UE_LOG(LogShibGameMode, Log, TEXT("%d players connected. %d players are expected. Waiting to start..."),ConnectedControllers.Num(), TempNumbOfPlayers);
			LobbyStatus = ELobbyStatus::WaitingToStart;
			UpdateConnectedCtrlGameRoom();
		}
	}
}

void AShibLobbyGameMode::HandleOnMatchReadyToStart()
{
	if (ADedicatedServerMatchmakingBeaconHost* BeaconFound = Cast<ADedicatedServerMatchmakingBeaconHost>(
		UGameplayStatics::GetActorOfClass(GetWorld(), ADedicatedServerMatchmakingBeaconHost::StaticClass())))
	{
		BeaconFound->OnMatchReadyToStart.RemoveAll(this);
	}

	if (RaceManager) RaceManager->SetExpectedNumbOfPlayers(GetNumPlayers());

	if (LobbyStatus != ELobbyStatus::WaitingToStart && LobbyStatus != ELobbyStatus::RaceIsStarting)
	{
		UE_LOG(LogShibGameMode, Log, TEXT("Matchmaking beacon: OnMatchReadyToStart: %d players connected. Waiting to start..."), GetNumPlayers());
		
		// this is just for a better flow
		// We don't change the lobby status from WaitingForPlayers to WaitingToStart if
		// the lobby status is about to change to RaceIsStarting anyway.
		if (LobbyTimeCounter>=5.f)
		{
			LobbyStatus = ELobbyStatus::WaitingToStart;
			UpdateConnectedCtrlGameRoom();
		}
	}
}
