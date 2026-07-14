// Copyright Shiba Inu Games LLC.

#include "ShibAttribute.h"
#include "ShibAttributeModifier.h"
#include "Net/UnrealNetwork.h"

void UShibAttribute::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(UShibAttribute, BaseValue, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UShibAttribute, CurrentValue, Params);
}

void UShibAttribute::InitAttribute()
{
	DefaultBaseValue = BaseValue;
	DefaultCurrentValue = CurrentValue;
}

bool UShibAttribute::AddModifier(TSubclassOf<UShibAttributeModifier> ModifierClass)
{
	if (!ensure(ModifierClass)) return false;

	auto* NewModifier = NewObject<UShibAttributeModifier>(this, ModifierClass);
	if (!ensure(NewModifier)) return false;

	Modifiers.Add(NewModifier);
	EModifierOperation Operation = NewModifier->GetOperation();

	if (NewModifier->IsModifyingBaseValue())
	{
		switch (Operation)
		{
		case EModifierOperation::Add:
			SetBaseValue(GetBaseValue() + NewModifier->ModifierValue);
			break;
		case EModifierOperation::Multiply:
			SetBaseValue(GetBaseValue() * NewModifier->ModifierValue);
			break;
		case EModifierOperation::Divide:
			SetBaseValue(GetBaseValue() / NewModifier->ModifierValue);
			break;
		case EModifierOperation::Subtract:
			SetBaseValue(GetBaseValue() - NewModifier->ModifierValue);
			break;
		}	
	}
	else
	{
		switch (Operation)
		{
		case EModifierOperation::Add:
			SetCurrentValue(GetCurrentValue() + NewModifier->ModifierValue);
			break;
		case EModifierOperation::Multiply:
			SetCurrentValue(GetCurrentValue() * NewModifier->ModifierValue);
			break;
		case EModifierOperation::Divide:
			SetCurrentValue(GetCurrentValue() / NewModifier->ModifierValue);
			break;
		case EModifierOperation::Subtract:
			SetCurrentValue(GetCurrentValue() - NewModifier->ModifierValue);
			break;
		}	
	}
	return true;
}

bool UShibAttribute::RemoveModifier(FGameplayTag ModifierTag, bool RemoveAll, bool ExactMatch)
{
	TArray<UShibAttributeModifier*> ModifiersToRemove;
	
	for (auto* Modifier : Modifiers)
	{
		if (!Modifier) continue;
		
		if (ExactMatch)
		{
			if (!Modifier->GetGameplayTag().MatchesTagExact(ModifierTag)) continue;
		}
		else
		{
			if (!Modifier->GetGameplayTag().MatchesTag(ModifierTag)) continue;
		}

		ModifiersToRemove.Add(Modifier);

		if (RemoveAll) continue;
		break;
	}

	if (!ModifiersToRemove.IsEmpty())
	{
		for (auto* ModifierToRemove: ModifiersToRemove)
		{
			Modifiers.Remove(ModifierToRemove);
		}

		// Recalculate the attribute current and base values with the new updated modifiers list
		ReevaluateAttribute();
		return true;
	}
	
	return false;
}

void UShibAttribute::ReevaluateAttribute()
{
	float TempBaseValue = DefaultBaseValue;
	float TempCurrentValue = DefaultCurrentValue;

	// Re calculate the base and current values with the current modifiers in the list
	for (auto* CurrentModifier : Modifiers)
	{
		EModifierOperation Operation = CurrentModifier->GetOperation();
		
		if (CurrentModifier->IsModifyingBaseValue())
		{
			switch (Operation)
			{
			case EModifierOperation::Add:
				TempBaseValue = (TempBaseValue + CurrentModifier->ModifierValue);
				break;
			case EModifierOperation::Multiply:
				TempBaseValue = (TempBaseValue * CurrentModifier->ModifierValue);
				break;
			case EModifierOperation::Divide:
				TempBaseValue = (TempBaseValue / CurrentModifier->ModifierValue);
				break;
			case EModifierOperation::Subtract:
				TempBaseValue = (TempBaseValue - CurrentModifier->ModifierValue);
				break;
			}	
		}
		else
		{
			switch (Operation)
			{
			case EModifierOperation::Add:
				TempCurrentValue = (TempCurrentValue + CurrentModifier->ModifierValue);
				break;
			case EModifierOperation::Multiply:
				TempCurrentValue = (TempCurrentValue * CurrentModifier->ModifierValue);
				break;
			case EModifierOperation::Divide:
				TempCurrentValue = (TempCurrentValue / CurrentModifier->ModifierValue);
				break;
			case EModifierOperation::Subtract:
				TempCurrentValue = (TempCurrentValue - CurrentModifier->ModifierValue);
				break;
			}	
		}
	}

	// If TempCurrentValue is equal to DefaultCurrentValue, it means the value isn't modified by any modifier anymore.
	// In that case, we want to reset the current to the base value with all the modifiers applied to the base value.
	// This saves us a function call to set the current value back to the base value down the road
	if (TempCurrentValue == DefaultCurrentValue)
	{
		TempCurrentValue = TempBaseValue;
	}
	
	SetBaseValue(TempBaseValue);
	SetCurrentValue(TempCurrentValue);
}

void UShibAttribute::OnRep_BaseValue(float OldValue)
{
	BaseValueChanged.Broadcast(OldValue, BaseValue);
}

void UShibAttribute::OnRep_CurrentValue(float OldValue)
{
	CurrentValueChanged.Broadcast(OldValue, CurrentValue);
}
