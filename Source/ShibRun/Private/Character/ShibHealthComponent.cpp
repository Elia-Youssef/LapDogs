// Copyright Shiba Inu Games LLC.

#include "Character/ShibHealthComponent.h"
#include "ShibAbility.h"
#include "ShibAbilityComponent.h"
#include "ShibAttribute.h"
#include "ShibAttributeSetComponent.h"
#include "GameplayTags/ShibGameplayTags.h"
#include "Net/UnrealNetwork.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShibHealthComponent, Log, All);
DEFINE_LOG_CATEGORY(LogShibHealthComponent);

// Sets default values for this component's properties
UShibHealthComponent::UShibHealthComponent()
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	AttributeSetComponent = nullptr;
	DeathState = EShibDeathState::NotDead;
	bOutOfHealth = false;
}

void UShibHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShibHealthComponent, DeathState);
}

void UShibHealthComponent::InitializeWithAbilitySystem(UShibAbilityComponent* InASC, UShibAttributeSetComponent* InAttSC)
{
	const AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogShibHealthComponent, Warning, TEXT("Health component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogShibHealthComponent, Error, TEXT("Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	AttributeSetComponent = InAttSC;
	if (!AttributeSetComponent)
	{
		UE_LOG(LogShibHealthComponent, Error, TEXT("Cannot initialize health component for owner [%s] with NULL attribute set."), *GetNameSafe(Owner));
		return;
	}

	if(!DeathAbility)
	{
		UE_LOG(LogShibHealthComponent, Error, TEXT("Cannot initialize health component for owner [%s] with NULL Death ability."), *GetNameSafe(Owner));
		return;	
	}

	// Set event to handle health change based on the Health attribute
	auto HealthAttr = AttributeSetComponent->GetAttribute(TAG_Attribute_ShibHealth);
	if (!HealthAttr)
	{
		UE_LOG(LogShibHealthComponent, Error, TEXT("Cannot initialize health component for owner [%s] with NULL attribute health in the attribute set."), *GetNameSafe(Owner));
		return;
	}

	HealthAttr->CurrentValueChanged.AddDynamic(this, &UShibHealthComponent::HandleHealthChanged);
	HealthAttr->BaseValueChanged.AddDynamic(this,&UShibHealthComponent::HandleMaxHealthChanged);

	OnHealthChanged.Broadcast(this, HealthAttr->GetBaseValue(), HealthAttr->GetBaseValue());
	OnMaxHealthChanged.Broadcast(this, HealthAttr->GetBaseValue(), HealthAttr->GetBaseValue());

	UE_LOG(LogShibHealthComponent, Log, TEXT("Health component for owner [%s] successfully initialized."), *GetNameSafe(Owner));
}

void UShibHealthComponent::UninitializeFromAbilitySystem()
{
	if (AttributeSetComponent)
	{
		if (auto HealthAttr = AttributeSetComponent->GetAttribute(TAG_Attribute_ShibHealth))
		{
			HealthAttr->CurrentValueChanged.RemoveAll(this);
			HealthAttr->BaseValueChanged.RemoveAll(this);
		}
	}
	
	OnDeathStarted.RemoveAll(this);
	OnDeathFinished.RemoveAll(this);
	
	AttributeSetComponent = nullptr;
	AbilitySystemComponent = nullptr;
}

float UShibHealthComponent::GetHealth() const
{
	if (AttributeSetComponent)
	{
		auto HealthAttr = AttributeSetComponent->GetAttribute(TAG_Attribute_ShibHealth);
		return HealthAttr ? HealthAttr->GetCurrentValue() : -1.0f;
	}
	
	return -1.0f;
}

float UShibHealthComponent::GetMaxHealth() const
{
	if (AttributeSetComponent)
	{
		auto HealthAttr = AttributeSetComponent->GetAttribute(TAG_Attribute_ShibHealth);
		return HealthAttr ? HealthAttr->GetBaseValue() : -1.0f;
	}
	
	return -1.0f;
}

float UShibHealthComponent::GetHealthNormalized() const
{
	if (AttributeSetComponent)
	{
		auto HealthAttr = AttributeSetComponent->GetAttribute(TAG_Attribute_ShibHealth);
		if (HealthAttr)
		{
			const float Health = HealthAttr->GetCurrentValue();
			const float MaxHealth = HealthAttr->GetBaseValue();

			return ((MaxHealth > 0.0f) ? (Health / MaxHealth) : 0.0f);	
		}
	}

	return 0.0f;
}

void UShibHealthComponent::StartDeath()
{
	if (DeathState != EShibDeathState::NotDead)
	{
		return;
	}
	
	DeathState = EShibDeathState::DeathStarted;
	
	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);
	
	Owner->ForceNetUpdate();
}

void UShibHealthComponent::FinishDeath()
{
	if (DeathState != EShibDeathState::DeathStarted)
	{
		return;
	}
	DeathState = EShibDeathState::DeathFinished;
	
	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);
	
	Owner->ForceNetUpdate();
}

void UShibHealthComponent::DamageSelfDestruct(bool bFellOutOfWorld)
{
	// TODO: if necessary
}

void UShibHealthComponent::ResetHealth()
{
	bOutOfHealth = false;
	DeathState = EShibDeathState::NotDead;
	
	// Reset the health value
	if (auto HealthAttr = AttributeSetComponent->GetAttribute(TAG_Attribute_ShibHealth))
	{
		// Remove damage modifiers to reset the health to its default value
		HealthAttr->RemoveModifier(TAG_Modifier_DamageType, true, false);
	}
	
	AActor* Owner = GetOwner();
	check(Owner);
	Owner->ForceNetUpdate();
}

void UShibHealthComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();
	
	Super::OnUnregister();
}

void UShibHealthComponent::HandleHealthChanged(float OldValue, float NewValue)
{
	AActor* Owner = GetOwner();
	check(Owner);

	// On Client, we just broadcast the new information
	if (!Owner->HasAuthority())
	{
		OnHealthChanged.Broadcast(this, OldValue, NewValue);
		return;
	}
	
	if (NewValue > 0.0f && !bOutOfHealth)
	{
		OnHealthChanged.Broadcast(this, OldValue, NewValue);
		return;
	}
	
	// If we're still alive and our health is zero or less, start death execution
	if (NewValue <= 0.0f && !bOutOfHealth)
	{
		// Notify one last time, but we just tell everybody that we now have 0 health
		OnHealthChanged.Broadcast(this, OldValue, 0.0f);

		bOutOfHealth = true;

		StartDeath();
		
		// Start death execution on the next tick to let time for the rest to finish executing
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "HandleOutOfHealth", GetOwner());
		GetWorld()->GetTimerManager().SetTimerForNextTick(Delegate);
	}
}

void UShibHealthComponent::HandleMaxHealthChanged(float OldValue, float NewValue)
{
	OnMaxHealthChanged.Broadcast(this, OldValue, NewValue);
}

void UShibHealthComponent::HandleOutOfHealth(AActor* OwningActor)
{
	AbilitySystemComponent->AddAbility(DeathAbility, OwningActor);
}

void UShibHealthComponent::OnRep_DeathState(EShibDeathState OldDeathState)
{
	const EShibDeathState NewDeathState = DeathState;
	
	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
	//DeathState = OldDeathState;

	//GEngine->AddOnScreenDebugMessage(-1, 10, FColor::Green, FString("OldDeathState: ") + *UEnum::GetValueAsString(OldDeathState) + FString(" - ") + FString("NewDeathState: ") + *UEnum::GetValueAsString(NewDeathState) );
	
	AActor* Owner = GetOwner();
	check(Owner);
	
	if (OldDeathState == EShibDeathState::NotDead)
	{
		if (NewDeathState == EShibDeathState::DeathStarted)
		{
			OnDeathStarted.Broadcast(Owner);
		}
		else if (NewDeathState == EShibDeathState::DeathFinished)
		{
			OnDeathStarted.Broadcast(Owner);
			OnDeathFinished.Broadcast(Owner);
		}
		else
		{
			UE_LOG(LogShibHealthComponent, Error, TEXT("Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == EShibDeathState::DeathStarted)
	{
		if (NewDeathState == EShibDeathState::DeathFinished || NewDeathState == EShibDeathState::NotDead)
		{
			OnDeathFinished.Broadcast(Owner);
		}
		else
		{
			UE_LOG(LogShibHealthComponent, Error, TEXT("Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
}
