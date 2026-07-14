// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BTT_Jump.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ShibCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"


EBTNodeResult::Type UBTT_Jump::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	const FVector JumpVector = ThisCharacter->GetCharacterMovement()->JumpZVelocity * ThisCharacter->GetActorUpVector();
	const FVector DestVector = Blackboard->GetValueAsVector(JumpDestinationSelector.SelectedKeyName) - ThisCharacter->GetActorLocation();
	const FVector LaunchVector = DestVector.GetSafeNormal() * ThisCharacter->GetCharacterMovement()->MaxWalkSpeed + JumpVector;
	
	ThisCharacter->SetActorRotation(ThisCharacter->GetVelocity().ToOrientationRotator()); // lerp this

	// Not sure if this will work but I had to cross out launch character since it wouldn't let me compile after I override it
	ThisCharacter->Jump();
	// ThisCharacter->LaunchCharacter(LaunchVector, true, true);

	Blackboard->SetValueAsBool(JumpSelector.SelectedKeyName, false);
	Blackboard->SetValueAsBool(MoveInAirSelector.SelectedKeyName, true);
	Blackboard->SetValueAsVector(MoveInAirDestinationSelector.SelectedKeyName, Blackboard->GetValueAsVector(JumpDestinationSelector.SelectedKeyName));

	return EBTNodeResult::Type::Succeeded;
}
