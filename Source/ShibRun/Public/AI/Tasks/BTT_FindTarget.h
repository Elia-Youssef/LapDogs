// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTask.h"
#include "BTT_FindTarget.generated.h"

class AShibAiWaypoint;

USTRUCT(BlueprintType)
struct FClosestActor
{
	GENERATED_BODY()

	UPROPERTY()
	FName Tag = FName();

	UPROPERTY()
	TObjectPtr<AActor> Actor = nullptr;

	UPROPERTY()
	float Distance = 0.f;
};

/**
 * 
 */
UCLASS()
class SHIBRUN_API UBTT_FindTarget : public UBaseTask
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetLocationSelector;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector TargetSelector;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector CurrentWaypointTargetSelector;
	
	UPROPERTY(EditAnywhere)
	FBlackboardKeySelector BranchIdSelector;

	UPROPERTY(EditAnywhere)
	TMap<FName, float> TagsPriority;

	UPROPERTY()
	TArray<AActor*> PreviousActors;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTargetActor;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	TArray<FClosestActor> GetClosestActorForEachTag();
	TObjectPtr<AActor> GetActorWithHighestPriority(TArray<FClosestActor>& ClosestActors);
	double GetPathLengthToActor(const FVector& Origin, const FVector& Target);
	FClosestActor GetClosestActorWithTag(const FName& Tag, const FVector& Origin);
};
