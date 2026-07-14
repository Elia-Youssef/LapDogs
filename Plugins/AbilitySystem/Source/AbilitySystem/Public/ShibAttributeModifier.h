// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "ShibAttributeModifier.generated.h"

UENUM(BlueprintType)
enum class EModifierOperation : uint8
{
	Add,
	Multiply,
	Divide,
	Subtract
};

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class ABILITYSYSTEM_API UShibAttributeModifier : public UObject
{
	GENERATED_BODY()
	
public:
	virtual bool IsSupportedForNetworking() const override { return true; }

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Modifier")
	FORCEINLINE FGameplayTag GetGameplayTag() const { return Tag; }

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Modifier")
	FORCEINLINE EModifierOperation GetOperation() const { return ModifierOperation; }

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Modifier")
	FORCEINLINE bool IsModifyingBaseValue() const { return bModifyBaseValue; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modifier")
	float ModifierValue;

private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier", meta = (AllowPrivateAccess = true))
	FName Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier", meta = (AllowPrivateAccess = true))
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier", meta = (AllowPrivateAccess = true))
	EModifierOperation ModifierOperation;

	// Weather or not this modifier modify the base value. If this is set to false, the modifier will be applied only to the current value
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Modifier", meta = (AllowPrivateAccess = true))
	bool bModifyBaseValue;
};
