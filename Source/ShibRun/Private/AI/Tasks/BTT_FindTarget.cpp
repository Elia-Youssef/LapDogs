// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTT_FindTarget.h"

#include "NavigationSystem.h"
#include "AI/Navigation/ShibAiWaypoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ShibCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

EBTNodeResult::Type UBTT_FindTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	if (!Blackboard) return EBTNodeResult::Type::Failed;

	// Get track waypoints
	TArray<AActor*> WaypointActors;
	UGameplayStatics::GetAllActorsOfClass(this, AShibAiWaypoint::StaticClass(), WaypointActors);
	
	const int32 CurrentWaypoint = Blackboard->GetValueAsInt(CurrentWaypointTargetSelector.SelectedKeyName);

	// Get Next and Last waypoints
	TArray<AShibAiWaypoint*> NextWaypoints;
	AShibAiWaypoint* LastWaypoint = nullptr;
	for (AActor* Waypoint : WaypointActors)
	{
		AShibAiWaypoint* ShibWaypoint = Cast<AShibAiWaypoint>(Waypoint);
		if (CurrentWaypoint == -1)
		{
			if (ShibWaypoint->MainRouteOrder == 0)
			{
				NextWaypoints.Add(ShibWaypoint);
			}
		} else if (ShibWaypoint->MainRouteOrder == CurrentWaypoint + 1)
		{
			NextWaypoints.Add(ShibWaypoint);
		}
		if (ShibWaypoint->MainRouteOrder == -1)
		{
			LastWaypoint = ShibWaypoint;
		}
	}

	AShibAiWaypoint* NextWaypoint = nullptr;
	// if a next point exists
	if (NextWaypoints.Num() > 0)
	{
		const FName CurrentBranchId = Blackboard->GetValueAsName(BranchIdSelector.SelectedKeyName);

		if (CurrentBranchId.IsNone())
		{
			// if we aren't walking on a specific branch, choose one randomly
			NextWaypoint = NextWaypoints[FMath::RandRange(0, NextWaypoints.Num() - 1)];
		} else
		{
			// if we're walking on a specific branch, choose waypoints from that branch
			for (auto* Wp : NextWaypoints)
			{
				if (Wp->BranchId == CurrentBranchId)
				{
					NextWaypoint = Wp;
					break;
				}
			}
	
			// if there are no waypoints on that specific branch anymore, choose a random one from the next waypoints
			if (!NextWaypoint)
			{
				NextWaypoint = NextWaypoints[FMath::RandRange(0, NextWaypoints.Num() - 1)];
			}
		}
	}
	if (NextWaypoint || LastWaypoint)
	{
		auto* Waypoint = NextWaypoint ? NextWaypoint : LastWaypoint;
		Blackboard->SetValueAsVector(TargetLocationSelector.SelectedKeyName, Waypoint->GetActorLocation());
		Blackboard->SetValueAsObject(TargetSelector.SelectedKeyName, Waypoint);
		Blackboard->SetValueAsInt(CurrentWaypointTargetSelector.SelectedKeyName, Waypoint->MainRouteOrder);
		Blackboard->SetValueAsName(BranchIdSelector.SelectedKeyName, Waypoint->BranchId);
	}

	TArray<FClosestActor> ClosestActors = GetClosestActorForEachTag();
	if (AActor* TargetActor = GetActorWithHighestPriority(ClosestActors))
	{
		if (IsValid(CurrentTargetActor)) PreviousActors.AddUnique(CurrentTargetActor);
		CurrentTargetActor = TargetActor;
	
		Blackboard->SetValueAsVector(TargetLocationSelector.SelectedKeyName, TargetActor->GetActorLocation());
		Blackboard->SetValueAsObject(TargetSelector.SelectedKeyName, TargetActor);
	}
	
	return EBTNodeResult::Type::Succeeded;
}

TObjectPtr<AActor> UBTT_FindTarget::GetActorWithHighestPriority(TArray<FClosestActor>& ClosestActors)
{
	AActor* TargetActor = nullptr;
	float HighestPriority = 0;
	
	for (auto& [Tag, Actor, Distance] : ClosestActors)
	{
		if (!IsValid(Actor)) continue;

		const float Priority = TagsPriority[Tag] * (1 / Distance + 0.01);
		if (Priority > HighestPriority)
		{
			HighestPriority = Priority;
			TargetActor = Actor;
		}
	}

	return TargetActor;
}

TArray<FClosestActor> UBTT_FindTarget::GetClosestActorForEachTag()
{
	TArray<FClosestActor> ClosestActors;
	
	for (auto& [Tag, Priority] : TagsPriority)
	{
		FClosestActor ClosestActor = GetClosestActorWithTag(Tag, ThisCharacter->GetActorLocation());
		ClosestActors.Add(ClosestActor);
	}

	return ClosestActors;
}

FClosestActor UBTT_FindTarget::GetClosestActorWithTag(const FName& Tag, const FVector& Origin)
{
	FClosestActor ClosestActor;
	ClosestActor.Tag = Tag;

	TArray<AActor*> ActorsWithTag;
	UGameplayStatics::GetAllActorsWithTag(ThisCharacter, Tag, ActorsWithTag);
	for (AActor* Actor : ActorsWithTag)
	{
		if (PreviousActors.Contains(Actor)) continue;

		const double PathLength = GetPathLengthToActor(Origin, Actor->GetActorLocation());
		if (!ClosestActor.Actor || (ClosestActor.Actor && PathLength < ClosestActor.Distance))
		{
			ClosestActor.Actor = Actor;
			ClosestActor.Distance = PathLength;
		}
	}

	return ClosestActor;
}

double UBTT_FindTarget::GetPathLengthToActor(const FVector& Origin, const FVector& Target)
{
	double PathLength;
	
	auto Result = UNavigationSystemV1::GetPathLength(ThisCharacter, Origin, Target, PathLength);

	if (Result != ENavigationQueryResult::Type::Success)
	{
		PathLength = UKismetMathLibrary::Vector_Distance(Origin, Target) * 10;
	}

	PathLength += FMath::Abs(Origin.Z - Target.Z) * 10;

	return PathLength;
}
