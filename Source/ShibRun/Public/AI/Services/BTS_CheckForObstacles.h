// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseService.h"
#include "BTS_CheckForObstacles.generated.h"

class AShibCharacter;
/**
 * 
 */
UCLASS()
class SHIBRUN_API UBTS_CheckForObstacles : public UBaseService
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector IsInAirSelector;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector PathIsValid;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	void IsInAir() const;
	void HasValidPath() const;
};
