// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Core/PushModel/PushModel.h"
#include "UObject/Object.h"
#include "ShibAttribute.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttributeBaseValueChangedDelegate, float, OldValue, float, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAttributeCurrentValueChangedDelegate, float, OldValue, float, NewValue);

class UShibAttributeModifier;

/**
 * 
 */
UCLASS(Blueprintable)
class ABILITYSYSTEM_API UShibAttribute : public UObject
{
	GENERATED_BODY()

public:
	virtual bool IsSupportedForNetworking() const override { return true; }

	// Init Attribute
	// For now it only saves the default current and base values
	virtual void InitAttribute();
	
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute")
	FORCEINLINE float GetBaseValue() const { return BaseValue; }

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute")
	FORCEINLINE float SetBaseValue(float NewBaseValue)
	{
		float Temp = BaseValue;
		BaseValue = NewBaseValue;
		BaseValueChanged.Broadcast(Temp, BaseValue);
		MARK_PROPERTY_DIRTY_FROM_NAME(UShibAttribute, BaseValue, this);
		return BaseValue;
	}

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute")
	FORCEINLINE float GetCurrentValue() const { return CurrentValue; }

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute")
	FORCEINLINE float SetCurrentValue(float NewCurrentValue)
	{
		float Temp = CurrentValue;
		CurrentValue = NewCurrentValue;
		CurrentValueChanged.Broadcast(Temp, CurrentValue);
		MARK_PROPERTY_DIRTY_FROM_NAME(UShibAttribute, CurrentValue, this);
		return CurrentValue;
	}

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute")
	FORCEINLINE FGameplayTag GetGameplayTag() const { return Tag; }

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute")
	bool AddModifier(TSubclassOf<UShibAttributeModifier> ModifierClass);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute")
	bool RemoveModifier(FGameplayTag ModifierTag, bool RemoveAll=true, bool ExactMatch=true);

	UPROPERTY(BlueprintAssignable, BlueprintReadWrite, Category = "Attribute")
	FAttributeBaseValueChangedDelegate BaseValueChanged;

	UPROPERTY(BlueprintAssignable, BlueprintReadWrite, Category = "Attribute")
	FAttributeBaseValueChangedDelegate CurrentValueChanged;

protected:
	void ReevaluateAttribute();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Attribute")
	FGameplayTag Tag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute", meta = (AllowPrivateAccess = true))
	FName Name;

	UPROPERTY(ReplicatedUsing = OnRep_BaseValue, EditAnywhere, Category = "Attribute")
	float BaseValue = 0;
	
	float DefaultBaseValue; // Keep the default base value in memory

	UPROPERTY(ReplicatedUsing = OnRep_CurrentValue, EditAnywhere, Category = "Attribute")
	float CurrentValue = 0;
	
	float DefaultCurrentValue; // Keep the default current value in memory
	
	UPROPERTY()
	TArray<UShibAttributeModifier*> Modifiers;
	
	UFUNCTION()
	void OnRep_BaseValue(float OldValue);

	UFUNCTION()
	void OnRep_CurrentValue(float OldValue);
};
