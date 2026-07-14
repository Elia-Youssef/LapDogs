// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BTS_CheckForObstacles.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/ShibCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Player/ShibAiController.h"


void UBTS_CheckForObstacles::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	IsInAir();
	HasValidPath();
}

void UBTS_CheckForObstacles::IsInAir() const
{
	Blackboard->SetValueAsBool(IsInAirSelector.SelectedKeyName, ThisCharacter->GetCharacterMovement()->IsFalling());
}

void UBTS_CheckForObstacles::HasValidPath() const
{
	Blackboard->SetValueAsBool(PathIsValid.SelectedKeyName, ThisController->GetPathFollowingComponent()->HasValidPath());
}
