// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseService.h"
#include "BTS_ReachedWaypoint.generated.h"

/**
 * 
 */
UCLASS()
class SHIBRUN_API UBTS_ReachedWaypoint : public UBaseService
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category="Reached Waypoint")
	FBlackboardKeySelector TargetSelector;
	
	UPROPERTY(EditAnywhere, Category="Reached Waypoint")
	FBlackboardKeySelector CurrentWaypointTargetSelector;
	
	UPROPERTY(EditAnywhere, Category="Reached Waypoint|Waypoint")
	FName WaypointTagName;
	
	UPROPERTY(EditAnywhere, Category="Reached Waypoint|Waypoint")
	uint32 DistanceToWaypoint;
	
	UPROPERTY(EditAnywhere, Category="Reached Waypoint|Endpoint")
	FName EndpointTagName;
	
	UPROPERTY(EditAnywhere, Category="Reached Waypoint|Endpoint")
	uint32 DistanceToEndpoint;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
