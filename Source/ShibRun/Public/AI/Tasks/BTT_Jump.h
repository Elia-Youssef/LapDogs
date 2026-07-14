// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Tasks/BaseTask.h"
#include "BTT_Jump.generated.h"

/**
 * 
 */
UCLASS()
class SHIBRUN_API UBTT_Jump : public UBaseTask
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector JumpSelector;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector JumpDestinationSelector;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector MoveInAirSelector;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector MoveInAirDestinationSelector;
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
