// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShibAi.generated.h"

#define SHIB_BLACKBOARD_KEY(Name) \
	inline FName Name = FName(#Name)

/**
 * These are the keys in the BB_NPC blackboard.
 * Use `FBlackboardKeySelector` where possible.
 * Update this namespace when updating the blackboard.
 */
namespace ShibBlackboardKeys
{
	SHIB_BLACKBOARD_KEY(TargetLocation);
	SHIB_BLACKBOARD_KEY(TargetActor);
	SHIB_BLACKBOARD_KEY(WaitToMove);
	SHIB_BLACKBOARD_KEY(EndpointReached);
	SHIB_BLACKBOARD_KEY(Jump);
	SHIB_BLACKBOARD_KEY(JumpDestination);
	SHIB_BLACKBOARD_KEY(IsInAir);
	SHIB_BLACKBOARD_KEY(ClosestActorInSight);
	SHIB_BLACKBOARD_KEY(MoveInAir);
	SHIB_BLACKBOARD_KEY(MoveInAirDestination);
	SHIB_BLACKBOARD_KEY(CurrentWaypointTarget);
	SHIB_BLACKBOARD_KEY(BranchId);
}

UCLASS()
class SHIBRUN_API UShibAi : public UObject
{
	GENERATED_BODY()

public:
};
