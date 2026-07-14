// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayTask/AT_CustomTick.h"

UAT_CustomTick::UAT_CustomTick(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
	bSimulatedTask = true;
}
UAT_CustomTick* UAT_CustomTick::CustomTickTask(const UObject* WorldContext, FName TaskInstanceName)
{
	UWorld* ContextWorld = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);
	if (!ensureAlwaysMsgf(IsValid(WorldContext), TEXT("World Context was not valid.")))
	{
		return nullptr;
	}
	UAT_CustomTick* MyObj = NewTask<UAT_CustomTick>(ContextWorld);
	return MyObj;
}

void UAT_CustomTick::Activate()
{
	Super::Activate();
}

void UAT_CustomTick::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	CustomTick(DeltaTime);
}

void UAT_CustomTick::CustomTick(float DeltaTime)
{
	OnTick.Broadcast(DeltaTime);
}

void UAT_CustomTick::OnDestroy(bool AbilityIsEnding)
{
	Super::OnDestroy(AbilityIsEnding);
	bTickingTask = false;
}
