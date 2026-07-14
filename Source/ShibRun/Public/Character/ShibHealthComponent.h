// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShibHealthComponent.generated.h"

class UShibAbilityComponent;
class UShibAttributeSetComponent;
class UShibAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShibDeathEvent, AActor*, OwningActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FShibHealthAttributeChanged, UShibHealthComponent*, HealthComponent, float, OldValue, float, NewValue);

/**
 * EShibDeathState
 *
 *	Defines current state of death.
 */
UENUM(BlueprintType)
enum class EShibDeathState : uint8
{
	NotDead = 0,
	DeathStarted,
	DeathFinished
};

UCLASS()
class SHIBRUN_API UShibHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UShibHealthComponent();

	// Returns the health component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "ShibCharacter|Health")
	static UShibHealthComponent* FindHealthComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UShibHealthComponent>() : nullptr); }

	// Initialize the component using an ability system component.
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter|Health")
	void InitializeWithAbilitySystem(UShibAbilityComponent* InASC, UShibAttributeSetComponent* InAttSC);

	// Uninitialize the component, clearing any references to the ability system.
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter|Health")
	void UninitializeFromAbilitySystem();

	// Returns the current health value.
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter|Health")
	float GetHealth() const;

	// Returns the current maximum health value.
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter|Health")
	float GetMaxHealth() const;

	// Returns the current health in the range [0.0, 1.0].
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter|Health")
	float GetHealthNormalized() const;

	UFUNCTION(BlueprintCallable, Category = "ShibCharacter|Health")
	EShibDeathState GetDeathState() const { return DeathState; }

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "ShibCharacter|Health", Meta = (ExpandBoolAsExecs = "ReturnValue"))
	bool IsDeadOrDying() const { return (DeathState > EShibDeathState::NotDead); }

	// Begins the death sequence for the owner.
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter|Health")
	virtual void StartDeath();

	// Ends the death sequence for the owner.
	UFUNCTION(BlueprintCallable, Category = "ShibCharacter|Health")
	virtual void FinishDeath();

	// Applies enough damage to kill the owner.
	virtual void DamageSelfDestruct(bool bFellOutOfWorld = false);

	UFUNCTION(BlueprintCallable)
	void ResetHealth();
	
public:
	// Delegate fired when the health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FShibHealthAttributeChanged OnHealthChanged;

	// Delegate fired when the max health value has changed. This is called on the client but the instigator may not be valid
	UPROPERTY(BlueprintAssignable)
	FShibHealthAttributeChanged OnMaxHealthChanged;

	// Delegate fired when the death sequence has started.
	UPROPERTY(BlueprintAssignable)
	FShibDeathEvent OnDeathStarted;

	// Delegate fired when the death sequence has finished.
	UPROPERTY(BlueprintAssignable)
	FShibDeathEvent OnDeathFinished;

protected:
	virtual void OnUnregister() override;
	
	UFUNCTION()
	void HandleHealthChanged(float OldValue, float NewValue);
	UFUNCTION()
	void HandleMaxHealthChanged(float OldValue, float NewValue);
	UFUNCTION()
	void HandleOutOfHealth(AActor* OwningActor);

	UFUNCTION()
	virtual void OnRep_DeathState(EShibDeathState OldDeathState);

protected:
	// Ability system component of the owner of this component
	UPROPERTY()
	TObjectPtr<UShibAbilityComponent> AbilitySystemComponent;

	// Attribute set component of the owner of this component
	UPROPERTY()
	TObjectPtr<UShibAttributeSetComponent> AttributeSetComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TSubclassOf<UShibAbility> DeathAbility;
	
private:
	// Replicated state used to handle dying.
	UPROPERTY(ReplicatedUsing = OnRep_DeathState)
	EShibDeathState DeathState;

	// Used to track when the health reaches 0.
	bool bOutOfHealth;
};
