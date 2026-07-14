// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BTS_ReachedWaypoint.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ShibCharacter.h"
#include "Kismet/KismetMathLibrary.h"

void UBTS_ReachedWaypoint::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (const AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetSelector.SelectedKeyName)))
	{
		auto From = ThisCharacter->GetActorLocation();
		auto To = TargetActor->GetActorLocation();
		From.Z = 0;
		To.Z = 0;

		const float DistanceToTarget = UKismetMathLibrary::Vector_Distance(From, To);

		if (DistanceToTarget <= DistanceToWaypoint)
		{
			Blackboard->SetValueAsObject(TargetSelector.SelectedKeyName, nullptr);
			
			if (TargetActor->ActorHasTag(EndpointTagName))
			{
				// do something here
			} else if (TargetActor->ActorHasTag(WaypointTagName))
			{
				// do something here
			}
		}
	}
}
