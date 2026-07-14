// Copyright Shiba Inu Games LLC.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShibObstacleManager.generated.h"

class AShibObstacle;

UCLASS()
class SHIBRUN_API AShibObstacleManager : public AActor
{
	GENERATED_BODY()

public:
	AShibObstacleManager();

	virtual void BeginPlay() override;
	
#pragma region Obstacles

	UPROPERTY(VisibleInstanceOnly)
	TArray<AShibObstacle*> Obstacles;
	
	UPROPERTY(EditAnywhere, Category="Timer")
	float Interval = 1.f;
	
	bool bObstacleState = false;

	FTimerHandle Timer_Obstacle;
	
	void StartObstacleTimer();

	UFUNCTION()
	void ObstacleTimer();

	void PopulateObstacles();

#pragma endregion Obstacles
	
};