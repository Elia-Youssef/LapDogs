// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibAbility.h"
#include "UObject/Object.h"
#include "ShibAbilityEffect.generated.h"

/** Enumeration outlining the possible gameplay effect magnitude calculation policies. */
UENUM()
enum class EEffectMagnitudeCalculation : uint8
{
	/** Use a simple, scalable float for the calculation. */
	ScalableFloat,
	/** Perform a calculation based upon an attribute. */
	AttributeBased,
	/** Perform a custom calculation, capable of capturing and acting on multiple attributes, in either BP or native. */
	//CustomCalculationClass,
	/** This magnitude will be set explicitly by the code/blueprint that creates the spec. */
	//SetByCaller,
};

/** Gameplay effect duration policies */
UENUM()
enum class EGameplayEffectDurationType : uint8
{
	/** This effect applies instantly */
	Instant,
	/** This effect lasts forever */
	Infinite,
	/** The duration of this effect will be specified by a magnitude */
	HasDuration
};

USTRUCT(BlueprintType)
struct ABILITYSYSTEM_API FEffectModifierDuration
{
	GENERATED_USTRUCT_BODY()

public:
	/** Default Constructor */
	FEffectModifierDuration()
		: DurationCalculationType(EEffectMagnitudeCalculation::ScalableFloat)
	{
	}

	/** Constructors for setting value in code (for automation tests) */
	FEffectModifierDuration(const float& Value)
		: DurationCalculationType(EEffectMagnitudeCalculation::ScalableFloat)
		, Duration(Value)
	{
	}
	FEffectModifierDuration(const FGameplayTag& Tag)
		: DurationCalculationType(EEffectMagnitudeCalculation::AttributeBased)
		, AttributeBasedTag(Tag)
	{
	}

	/** 
	* Attempts to calculate the duration given the provided settings. May fail if necessary information (such as captured attributes) is missing 
	*/
	bool CalculateDuration(const AActor* Instigator, OUT float& OutCalculatedDuration);

	FORCEINLINE float GetDuration() { return Duration; }

	bool operator==(const FEffectModifierDuration& Other) const;
	bool operator!=(const FEffectModifierDuration& Other) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = Duration)
	EEffectMagnitudeCalculation DurationCalculationType;

	UPROPERTY(EditDefaultsOnly, Category="Duration", meta=(EditCondition="DurationCalculationType == EEffectMagnitudeCalculation::AttributeBased", EditConditionHides))
	FGameplayTag AttributeBasedTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Duration", meta=(EditCondition="DurationCalculationType == EEffectMagnitudeCalculation::ScalableFloat", EditConditionHides))
	float Duration{0};

	friend class UShibAbilityEffect;
};

/**
 * 
 */
UCLASS(hidecategories = ("Cooldown", "Ability|Automation"))
class ABILITYSYSTEM_API UShibAbilityEffect : public UShibAbility
{
	GENERATED_BODY()

public:
	UShibAbilityEffect();
	virtual bool CanStart_Implementation(AActor* Instigator) override;
	virtual bool Start_Implementation(AActor* Instigator) override;
	virtual bool Stop_Implementation(AActor* Instigator) override;
	virtual bool Cancel_Implementation(AActor* Instigator) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Effect", meta=(EditCondition="DurationPolicy == EGameplayEffectDurationType::HasDuration", EditConditionHides))
	FEffectModifierDuration DurationType;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	EGameplayEffectDurationType DurationPolicy;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", meta=(EditCondition="DurationPolicy != EGameplayEffectDurationType::Instant", EditConditionHides))
	float Period;

	FTimerHandle PeriodHandle;
	FTimerHandle DurationHandle;
	
	// Set this to true to sent notifications to the owner of this ability at each state changes (Start, Stop, Cancel, Cooldown)
	// Server ONLY!
	UPROPERTY(EditAnywhere, Category = "Effect|Automation")
	bool bEffectNotifyOwner;

	// Set this to true to sent notifications to everybody at each state changes (Start, Stop, Cancel, Cooldown)
	// The notification will be sent where the function is executed (Server, owning client and/or other clients).
	UPROPERTY(EditAnywhere, Category = "Effect|Automation")
	bool bEffectNotifyAll;

	UFUNCTION(BlueprintNativeEvent, Category = "Effect")
	void ExecutePeriodicEffect(AActor* Instigator);
};
