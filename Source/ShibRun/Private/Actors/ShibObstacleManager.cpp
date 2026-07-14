#include "Actors/ShibObstacleManager.h"

#include "Actors/ShibObstacle.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShibController.h"

AShibObstacleManager::AShibObstacleManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AShibObstacleManager::BeginPlay()
{
	Super::BeginPlay();

	PopulateObstacles();
	StartObstacleTimer();
}

#pragma region Obstacles

void AShibObstacleManager::StartObstacleTimer()
{
	if( ! HasAuthority() || Obstacles.IsEmpty()) {return;}
	
	if(GetWorld())
	{
		GetWorldTimerManager().SetTimer(Timer_Obstacle, this, &AShibObstacleManager::ObstacleTimer, Interval, true, 2.f);
	}
}

void AShibObstacleManager::ObstacleTimer()
{
	bObstacleState = !bObstacleState;

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, AShibController::StaticClass(), FoundActors);
	for(const auto Actor : FoundActors)
	{
		if(const AShibController* ShibController = Cast<AShibController>(Actor))
		{
			ShibController->Client_ObstacleChangeState(bObstacleState);
		}
	}
	
	// Authority has to toggle traps in game mode (in case of dedicated server). Player controller call is ignored on authority
	for(const auto Obstacle : Obstacles)
	{
		Obstacle->StateChanged(bObstacleState);
	}
}

void AShibObstacleManager::PopulateObstacles()
{
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, AShibObstacle::StaticClass(), FoundActors);
	for(const auto Obstacle : FoundActors)
	{
		if(AShibObstacle* ShibObstacle = Cast<AShibObstacle>(Obstacle))
		{
			Obstacles.Add(ShibObstacle);
		}
	}
}

#pragma endregion Obstacles
