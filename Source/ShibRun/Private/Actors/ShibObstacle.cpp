// Copyright Shiba Inu Games LLC.
#include "Actors/ShibObstacle.h"

#include "Actors/ShibObstacleManager.h"
#include "Kismet/GameplayStatics.h"

void AShibObstacle::BeginPlay()
{
	Super::BeginPlay();

	if( ! HasAuthority()){return;}
	
	// Create the obstacle manager if it doesn't exist
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> FoundManagers;
		UGameplayStatics::GetAllActorsOfClass(World, AShibObstacleManager::StaticClass(), FoundManagers);

		if (FoundManagers.IsEmpty())
		{
			const FActorSpawnParameters SpawnParams;
			if (AShibObstacleManager* ObstacleManager = World->SpawnActor<AShibObstacleManager>(AShibObstacleManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams))
			{
				
			}
		}
	}
}

void AShibObstacle::StateChanged(bool bNewState)
{
	OnStateChanged(bNewState);
}
