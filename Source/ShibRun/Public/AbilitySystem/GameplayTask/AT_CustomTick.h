// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "AT_CustomTick.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCustomTickDelegate, float, DeltaTime);
 
UCLASS()
class SHIBRUN_API UAT_CustomTick : public UGameplayTask
{
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(BlueprintAssignable)
	FCustomTickDelegate	OnTick;
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks" )
	static UAT_CustomTick* CustomTickTask(const UObject* WorldContext, FName TaskInstanceName);

	virtual void Activate() override;
	
	virtual void TickTask(float DeltaTime) override;
	
	void CustomTick(float DeltaTime);

	virtual void OnDestroy(bool AbilityIsEnding) override;
};
