// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Tasks/BaseTask.h"
#include "BTT_MoveInAir.generated.h"

/**
 * 
 */
UCLASS()
class SHIBRUN_API UBTT_MoveInAir : public UBaseTask
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector Jump;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector IsInAir;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector MoveInAir;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetLocation;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector MoveInAirDestination;
	
	UPROPERTY(EditAnywhere)
	float MoveInAirAcceptanceRadius = 20.f;
	
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
