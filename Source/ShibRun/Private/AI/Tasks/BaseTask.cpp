// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Tasks/BaseTask.h"

#include "BlueprintNodeHelpers.h"
#include "Character/ShibCharacter.h"
#include "Player/ShibAiController.h"

UBaseTask::UBaseTask()
{
	bCreateNodeInstance = true;
	
	const UClass* StopAtClass = UBaseTask::StaticClass();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		BlueprintNodeHelpers::CollectPropertyData(this, StopAtClass, PropertyData);
	}
}

EBTNodeResult::Type UBaseTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!Blackboard) Blackboard = OwnerComp.GetBlackboardComponent();
	if (!ThisCharacter) ThisCharacter = Cast<AShibCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	if (!ThisController) ThisController = Cast<AShibAiController>(OwnerComp.GetAIOwner());
	
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

FString UBaseTask::GetStaticDescription() const
{
	FString ReturnDesc;

	if (const UBaseTask* CDO = static_cast<UBaseTask*>(GetClass()->GetDefaultObject()))
	{
		const UClass* StopAtClass = UBaseTask::StaticClass();
		
		FString PropertyDesc = BlueprintNodeHelpers::CollectPropertyDescription(this, StopAtClass, CDO->PropertyData);
		
		if (PropertyDesc.Len()) ReturnDesc += PropertyDesc;
	}

	return ReturnDesc;
}
