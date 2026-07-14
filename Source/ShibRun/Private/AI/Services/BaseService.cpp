// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Services/BaseService.h"

#include "AIController.h"
#include "BlueprintNodeHelpers.h"
#include "Character/ShibCharacter.h"
#include "Player/ShibAiController.h"

UBaseService::UBaseService()
{
	bCreateNodeInstance = true;

	const UClass* StopAtClass = UBaseService::StaticClass();
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		BlueprintNodeHelpers::CollectPropertyData(this, StopAtClass, PropertyData);
	}
}

void UBaseService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (!Blackboard) Blackboard = OwnerComp.GetBlackboardComponent();
	if (!ThisCharacter) ThisCharacter = Cast<AShibCharacter>(OwnerComp.GetAIOwner()->GetCharacter());
	if (!ThisController) ThisController = Cast<AShibAiController>(OwnerComp.GetAIOwner());
}

FString UBaseService::GetStaticServiceDescription() const
{
	FString ReturnDesc;

	if (UBaseService* CDO = static_cast<UBaseService*>(GetClass()->GetDefaultObject()))
	{
		ReturnDesc += FString::Printf(TEXT("%s\n\n"), *Super::GetStaticServiceDescription());

		const UClass* StopAtClass = UBaseService::StaticClass();
		FString PropertyDesc = BlueprintNodeHelpers::CollectPropertyDescription(this, StopAtClass, CDO->PropertyData);
		if (PropertyDesc.Len())
		{
			ReturnDesc += PropertyDesc;
		}
	}

	return ReturnDesc;
}
