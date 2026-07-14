// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "AI/Tasks/BaseTask.h"
#include "BTT_MoveForward.generated.h"

/**
 * 
 */
UCLASS()
class SHIBRUN_API UBTT_MoveForward : public UBaseTask
{
	GENERATED_BODY()

public:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
