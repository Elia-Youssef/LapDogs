// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ShibBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SHIBRUN_API UShibBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Zorans Blueprint Function Library")
	static EShibDirection4 GetActorDirection4(const AActor* Actor, const FVector& Direction);
};
