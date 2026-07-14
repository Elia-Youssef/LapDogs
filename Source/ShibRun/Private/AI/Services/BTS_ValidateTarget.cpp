// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BTS_ValidateTarget.h"

#include "BehaviorTree/BlackboardComponent.h"

void UBTS_ValidateTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const AActor* TargetActor = Cast<AActor>(Blackboard->GetValueAsObject(TargetSelector.SelectedKeyName));
	
	if (!IsValid(TargetActor))
	{
		Blackboard->SetValueAsObject(TargetSelector.SelectedKeyName, nullptr);
	}
}
