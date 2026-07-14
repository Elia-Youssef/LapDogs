// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BaseService.generated.h"

class AShibAiController;
class AShibCharacter;
/**
 * 
 */
UCLASS()
class SHIBRUN_API UBaseService : public UBTService
{
	GENERATED_BODY()

public:
	UBaseService();
	TArray<FProperty*> PropertyData;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticServiceDescription() const override;

	UPROPERTY()
	UBlackboardComponent* Blackboard = nullptr;

	UPROPERTY()
	AShibCharacter* ThisCharacter = nullptr;

	UPROPERTY()
	AShibAiController* ThisController = nullptr;
};
