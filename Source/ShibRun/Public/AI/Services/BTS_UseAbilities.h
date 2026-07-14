// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/Services/BaseService.h"
#include "BTS_UseAbilities.generated.h"


class AShibPlayerState;
/**
 * 
 */
UCLASS(BlueprintType)
class SHIBRUN_API UBTS_UseAbilities : public UBaseService
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float ChanceToUseAbility = 5.f;

	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector ClosestActorInSightSelector;
	
	UPROPERTY()
	AShibPlayerState* ShibPS;
	
	UPROPERTY()
	TObjectPtr<AActor> ClosestActorInSight;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	void UsePrimeAbilities();
	void UseRyoAbilities();
	void UseWagmiAbilities();
	void UseRocketPondAbilities();

private:
	void AimAtTarget();
};
