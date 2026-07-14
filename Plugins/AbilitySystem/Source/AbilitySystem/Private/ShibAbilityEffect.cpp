// Copyright Shiba Inu Games LLC.

#include "ShibAbilityEffect.h"
#include "ShibAbilityComponent.h"
#include "ShibAttributeSetComponent.h"
#include "TimerManager.h"

UShibAbilityEffect::UShibAbilityEffect()
{
	bAutoStart = true;
	bAutoStop = false;
	bAutoStartCooldown = false;
	bAutoRemove = true;
	bEffectNotifyAll = false;
	bEffectNotifyOwner = false;
	DurationPolicy = EGameplayEffectDurationType::Instant;
	Period = 0.f;

	NetExecution = ENetExecution::NE_NotReplicated;
}

bool UShibAbilityEffect::Start_Implementation(AActor* Instigator)
{
	bool ret = Super::Start_Implementation(Instigator);

	if (DurationPolicy == EGameplayEffectDurationType::HasDuration)
	{
		if (DurationType.CalculateDuration(Instigator, DurationType.Duration))
		{
			if (DurationType.Duration > 0.0f)
			{
				FTimerDelegate Delegate;
				Delegate.BindUFunction(this, "Stop", Instigator);
				GetWorld()->GetTimerManager().SetTimer(DurationHandle, Delegate, DurationType.Duration, false);
			}
		}
	}

	if (bEffectNotifyOwner && GetOwnerHasAutority())
	{
		GetOwningComponent()->Client_BroadcastAbilityStarted(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator, DurationType.Duration);
	}

	if (bEffectNotifyAll)
	{
		GetOwningComponent()->AbilityStartedDelegate.Broadcast(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator, DurationType.Duration);
	}

	if (DurationPolicy == EGameplayEffectDurationType::Instant)
	{
		return Stop(Instigator) && ret;
	}

	// Setup periodic event if the duration is not equal to zero
	// Only effects with a duration (or infinite duration) can have a periodic effect
	if (Period > 0.0f)
	{
		FTimerDelegate Delegate;
		Delegate.BindUFunction(this, "ExecutePeriodicEffect", Instigator);
		GetWorld()->GetTimerManager().SetTimer(PeriodHandle, Delegate, Period, true);
	}

	return ret;
}

bool UShibAbilityEffect::Stop_Implementation(AActor* Instigator)
{
	if (PeriodHandle.IsValid())
	{
		if (GetWorld()->GetTimerManager().GetTimerRemaining(PeriodHandle) < UE_KINDA_SMALL_NUMBER)
		{
			ExecutePeriodicEffect(Instigator);
		}
	}
	
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (bEffectNotifyOwner && GetOwnerHasAutority())
	{
		GetOwningComponent()->Client_BroadcastAbilityStopped(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator);
	}

	if (bEffectNotifyAll)
	{
		GetOwningComponent()->AbilityStoppedDelegate.Broadcast(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator);
	}
	
	return Super::Stop_Implementation(Instigator);
}

bool UShibAbilityEffect::Cancel_Implementation(AActor* Instigator)
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (bEffectNotifyOwner && GetOwnerHasAutority())
	{
		GetOwningComponent()->Client_BroadcastAbilityCancelled(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator);
	}

	if (bEffectNotifyAll)
	{
		GetOwningComponent()->AbilityCancelledDelegate.Broadcast(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator);
	}
	
	return Super::Cancel_Implementation(Instigator);
}

bool UShibAbilityEffect::CanStart_Implementation(AActor* Instigator)
{
	auto* Comp = GetOwningComponent();
	if (!ensure(Comp)) return false;

	const bool bSuccess = Super::CanStart_Implementation(Instigator);
	
	if(!bSuccess)
	{
		Comp->RemoveAbility(AbilityTag);
	}

	return bSuccess;
}

void UShibAbilityEffect::ExecutePeriodicEffect_Implementation(AActor* Instigator)
{
   //BlueprintNativeEvent, but we could add generic functionalities here at some point. TBD
}

bool FEffectModifierDuration::CalculateDuration(const AActor* Instigator, OUT float& OutCalculatedDuration)
{
	switch (DurationCalculationType)
	{
	case EEffectMagnitudeCalculation::ScalableFloat:
		{
			OutCalculatedDuration = Duration;
			break;
		}
	case EEffectMagnitudeCalculation::AttributeBased:
		{
			auto* AttributeComp = Cast<UShibAttributeSetComponent>(Instigator->GetComponentByClass(UShibAttributeSetComponent::StaticClass()));
			if (!AttributeComp) return false;
			AttributeComp->GetAttributeCurrent(AttributeBasedTag, OutCalculatedDuration);
			break;	
		}
	}
	return true;
}
