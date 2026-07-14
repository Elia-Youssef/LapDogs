// Copyright Shiba Inu Games LLC.

#include "AbilitySystem/ShibAbilitySystemComponent.h"
#include "ShibAbility.h"
#include "GameplayTags/ShibGameplayTags.h"

UShibAbility* UShibAbilitySystemComponent::AddAbility(TSubclassOf<UShibAbility> AbilityClass, AActor* Instigator)
{
	if (!ensure(AbilityClass)) return nullptr;
	const UShibAbility* DefaultAbility = AbilityClass->GetDefaultObject<UShibAbility>();

	if( ! GetOwner()->HasAuthority() && ! DefaultAbility->IsNetExecution(ENetExecution::NE_NotReplicated))
	{
		// Abilities that are not NotReplicated are always instantiated and replicated on server only
		ServerAddAbility(AbilityClass, Instigator);
		return nullptr;
	}
	
	// We want only one single pickup ability at any point, so we make sure we don't already have one before adding it
	if (DefaultAbility->GetGameplayTag().MatchesTag(TAG_PickupAbility))
	{
		UShibAbility* AbilityAlreadyAddedFound = nullptr;
		if (FindAndGetOwnedAbility(TAG_PickupAbility, false, AbilityAlreadyAddedFound))
		{
			// If we already have one we remove it, we can add the new one after if we successfully removed the old one
			if (RemoveAbility(AbilityAlreadyAddedFound->GetGameplayTag()))
			{
				return Super::AddAbility(AbilityClass, Instigator);
			}
			return nullptr;
		}
	}
	else if (DefaultAbility->GetGameplayTag().MatchesTag(TAG_Effect_Buff_SpeedBoost))
	{
		UShibAbility* AbilityAlreadyAddedFound = nullptr;
		if (FindAndGetOwnedAbility(TAG_Effect_Buff_SpeedBoost, false, AbilityAlreadyAddedFound))
		{
			// If we already have one we remove it, we can add the new one after if we successfully removed the old one
			if (CancelAbility(AbilityAlreadyAddedFound->GetGameplayTag(), Instigator))
			{
				return Super::AddAbility(AbilityClass, Instigator);
			}
			return nullptr;
		}	
	}
	else if (DefaultAbility->GetGameplayTag().MatchesTag(TAG_Effect_Cooldown)) // When we already have a cooldown effect and we add another one of the same class, stop and add again the same effect to restart cooldown
	{
		UShibAbility* AbilityAlreadyAddedFound = nullptr;
		if (FindAndGetOwnedAbility(DefaultAbility->GetGameplayTag(), true, AbilityAlreadyAddedFound))
		{
			// If we already have one we remove it, we can add the new one after if we successfully removed the old one
			if (StopAbility(AbilityAlreadyAddedFound->GetGameplayTag(), Instigator))
			{
				return Super::AddAbility(AbilityClass, Instigator);
			}
			return nullptr;
		}	
	}
	
	return Super::AddAbility(AbilityClass, Instigator);
}

void UShibAbilitySystemComponent::OnAbilityPickupAdded(FGameplayTag AbilityTag, AActor* Owner)
{
	AbilityPickupAddedDelegate.Broadcast(AbilityTag, Owner);
}

void UShibAbilitySystemComponent::OnAbilityPickupRemoved(FGameplayTag AbilityTag, AActor* Owner)
{
	AbilityPickupRemovedDelegate.Broadcast(AbilityTag, Owner);
}


