// Copyright Shiba Inu Games LLC.

#include "AI/Navigation/ShibAiWaypoint.h"
#include "EngineUtils.h"

void AShibAiWaypoint::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	UWorld* World = GetWorld();
	if (!World) return;

	TArray<AShibAiWaypoint*> LastWaypoints;
	int32 HighestWaypointNumb=0;

	for (TActorIterator<AShibAiWaypoint> It(World); It; ++It)
	{
		AShibAiWaypoint* Waypoint = *It;
		
		bool bSameBranchId = bSplitHere || bMergeHere ?  true : Waypoint->BranchId == BranchId;
		
		if (HighestWaypointNumb < Waypoint->MainRouteOrder) HighestWaypointNumb = Waypoint->MainRouteOrder;
		if (Waypoint->MainRouteOrder == -1 && bSameBranchId) LastWaypoints.AddUnique(Waypoint);
		
		if (MainRouteOrder == -1)
		{
			if (Waypoint->MainRouteOrder == 0 && bSameBranchId)
			{
				NextWaypoints.AddUnique(Waypoint);
			}
		} else if (Waypoint->MainRouteOrder == MainRouteOrder + 1 && bSameBranchId)
		{
			NextWaypoints.AddUnique(Waypoint);
		}
	}

	// If we are the last waypoint, we should still have an empty array at this point
	// So we linked it to the last one
	if (NextWaypoints.IsEmpty() && HighestWaypointNumb == MainRouteOrder)
	{
		NextWaypoints.Append(LastWaypoints);
	}
}
