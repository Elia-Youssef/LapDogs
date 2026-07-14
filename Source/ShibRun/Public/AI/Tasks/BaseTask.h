// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BaseTask.generated.h"

class AShibAiController;
class AShibCharacter;
/**
 * 
 */
UCLASS()
class SHIBRUN_API UBaseTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBaseTask();
	TArray<FProperty*> PropertyData;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;

	UPROPERTY()
	UBlackboardComponent* Blackboard = nullptr;

	UPROPERTY()
	AShibCharacter* ThisCharacter = nullptr;

	UPROPERTY()
	AShibAiController* ThisController = nullptr;
};
