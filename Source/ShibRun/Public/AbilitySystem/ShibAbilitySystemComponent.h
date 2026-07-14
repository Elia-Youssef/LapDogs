// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShibAbilityComponent.h"
#include "ShibAbilitySystemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAbilityPickupNotify, const FGameplayTag, Tag, const AActor*, Owner);

UCLASS()
class SHIBRUN_API UShibAbilitySystemComponent : public UShibAbilityComponent
{
	GENERATED_BODY()

public:
	virtual UShibAbility* AddAbility(TSubclassOf<UShibAbility> AbilityClass, AActor* Instigator) override;

	UFUNCTION(BlueprintCallable)
	void OnAbilityPickupAdded(FGameplayTag AbilityTag, AActor* Owner);

	UFUNCTION(BlueprintCallable)
	void OnAbilityPickupRemoved(FGameplayTag AbilityTag, AActor* Owner);
	
	UPROPERTY(BlueprintAssignable)
	FAbilityPickupNotify AbilityPickupAddedDelegate;

	UPROPERTY(BlueprintAssignable)
	FAbilityPickupNotify AbilityPickupRemovedDelegate;
};
