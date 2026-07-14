// Copyright Shiba Inu Games LLC.

#include "Player/ShibController.h"
#include "Actors/ShibObstacle.h"
#include "Engine/OverlapResult.h"
#include "Game/ShibBaseGameMode.h"
#include "GameFramework/GameStateBase.h"

void AShibController::BeginPlay()
{
	Super::BeginPlay();
}

void AShibController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	OnRepPlayerState();
}

void AShibController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
}

void AShibController::Server_RequestRoundTripTime_Implementation(APlayerController* requester, float ClientTime)
{
	float serverTime = GetWorld()->GetGameState()->GetServerWorldTimeSeconds();
	Client_ReportRoundTripTime(ClientTime, serverTime);
}

bool AShibController::Server_RequestRoundTripTime_Validate(APlayerController* requester, float requestWorldTime)
{
	return true;
}

void AShibController::Client_ReportRoundTripTime_Implementation(float ClientTime, float ServerTime)
{
	// We divide RTT by two because it's the time it takes to go from client to server and back from server to client
	// Obviously, RTT / 2 isn't representative of all networking conditions since it assumes that RTT is perfectly split 50/50.
	// Better than nothing.
	float ClientReceptionTime = GetWorld()->GetTimeSeconds();
	float RoundTripTime = (ClientReceptionTime - ClientTime) / 2;

	ServerRoundTripTimeBuffer.Add(RoundTripTime);
	UE_LOG(LogTemp, Log, TEXT("RTT request No.%d finished for %s with value: %f."),ServerRoundTripTimeBuffer.Num(), *GetNameSafe(this), RoundTripTime);
	
	// Get a sample of 10 values
	if (ServerRoundTripTimeBuffer.Num() < 10)
	{
		Server_RequestRoundTripTime(this, ClientReceptionTime);
		return;
	}

	// Sort the array
	ServerRoundTripTimeBuffer.Sort();
	// Calculate sum of the buffer array
	// Here we ignore the first two and last two values to exclude potential aberrant values
	float RttSum = 0;
	for(int32 i=0; i<ServerRoundTripTimeBuffer.Num(); i++)
	{
		if (i<2||i>=ServerRoundTripTimeBuffer.Num()-2)
		{
			continue;
		}
		RttSum += ServerRoundTripTimeBuffer[i];
	}
	
	ServerRoundTripTime = RttSum/(ServerRoundTripTimeBuffer.Num()-4); // we use the average value from the buffer array as our final RTT value
	//Empty the buffer array to be ready for next time
	ServerRoundTripTimeBuffer.Empty();
	UE_LOG(LogTemp, Log, TEXT("RTT updated for %s with the value: %f"), *GetNameSafe(this), ServerRoundTripTime);
	OnRoundTripTimeUpdatedDelegate.Broadcast();
}

#pragma region Obstacles

void AShibController::Client_ObstacleChangeState_Implementation(bool bNewState) const
{
	// Authority already toggles the traps in game mode (in case of dedicated server)
	if(HasAuthority()){return;}
	
	TArray<AActor*> FoundActors = GetNearbyActorsOfClass(AActor::StaticClass(), 8000.f); // 8000 is the range we will update traps from player pawn
	for(const auto Actor : FoundActors)
	{
		if(AShibObstacle* Obstacle = Cast<AShibObstacle>(Actor))
		{
			Obstacle->StateChanged(bNewState);
		}
	}
}

TArray<AActor*> AShibController::GetNearbyActorsOfClass(UClass* ActorClass, float Range) const
{
	TArray<AActor*> NearbyActors;

	if(const APawn* PlayerPawn = GetPawn())
	{
		const FVector PlayerLocation = PlayerPawn->GetActorLocation();
		
		TArray<FOverlapResult> OverlapResults;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(PlayerPawn); 
		
		if (GetWorld()->OverlapMultiByObjectType(OverlapResults,PlayerLocation,FQuat::Identity,FCollisionObjectQueryParams(ECC_WorldDynamic),FCollisionShape::MakeSphere(Range),QueryParams))
		{
			for (const FOverlapResult& Result : OverlapResults)
			{
				AActor* OverlappedActor = Result.GetActor();
				if (OverlappedActor && OverlappedActor->IsA(ActorClass))
				{
					NearbyActors.Add(OverlappedActor);
				}
			}
		}
	}
	
	return NearbyActors;
}

#pragma endregion Obstacles

void AShibController::Client_OnMatchStateChangeNotify_Implementation(FName CurrentMatchState, float MatchStateTime)
{
	// Notify any local system that the match state changes
	// Mainly used to set the match timer in the HUD
	OnMatchStateChanged.Broadcast(CurrentMatchState, MatchStateTime);
	
	if (CurrentMatchState == MatchState::Countdown)
	{
		OnRaceWarmup(MatchStateTime);
	}
	else if (CurrentMatchState == MatchState::InProgress)
	{
		OnRaceStart(MatchStateTime);
	}
	else if (CurrentMatchState == MatchState::EndPending)
	{
		OnRacePendingEnd(MatchStateTime);
	}
	else if (CurrentMatchState == MatchState::PlayerFinishedRace)
	{
		OnPlayerFinishedRace();
	}
	else if (CurrentMatchState == MatchState::WaitingPostMatch)
	{
		OnRaceEnd();
	}
}