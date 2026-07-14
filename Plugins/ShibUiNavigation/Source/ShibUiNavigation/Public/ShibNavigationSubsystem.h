// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShibNavigationSubsystem.generated.h"

struct FNavigationHistory;

UCLASS()
class SHIBUINAVIGATION_API UShibNavigationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TArray<FNavigationHistory> NavigationHistory;
};
