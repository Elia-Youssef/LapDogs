// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTT_MoveInAir.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ShibCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

EBTNodeResult::Type UBTT_MoveInAir::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	
	if (!ThisCharacter->GetCharacterMovement()->IsFalling())
	{
		Blackboard->SetValueAsBool(IsInAir.SelectedKeyName, false);
		Blackboard->SetValueAsBool(MoveInAir.SelectedKeyName, false);
		return EBTNodeResult::Type::Succeeded;
	}

	Blackboard->SetValueAsBool(Jump.SelectedKeyName, false);

	// if MoveInAir, then we have a location to move to (MoveInAirDestination)
	// else, move to the target location
	FVector Destination;
	if (Blackboard->GetValueAsBool(MoveInAir.SelectedKeyName))
	{
		Destination = Blackboard->GetValueAsVector(MoveInAirDestination.SelectedKeyName);

		if ((Destination - ThisCharacter->GetActorLocation()).Length() < MoveInAirAcceptanceRadius)
		{
			Blackboard->SetValueAsBool(MoveInAir.SelectedKeyName, false);
			return EBTNodeResult::Type::Succeeded;
		}
	} else
	{
		Destination = Blackboard->GetValueAsVector(TargetLocation.SelectedKeyName);
	}
	
	const FVector MoveTo = (Destination - ThisCharacter->GetActorLocation()).GetSafeNormal();

	ThisCharacter->AddMovementInput(MoveTo, 1, true);
	
	return EBTNodeResult::Type::Succeeded;
}
