// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseService.h"
#include "BTS_ValidateTarget.generated.h"

/**
 * 
 */
UCLASS()
class SHIBRUN_API UBTS_ValidateTarget : public UBaseService
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetSelector;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
