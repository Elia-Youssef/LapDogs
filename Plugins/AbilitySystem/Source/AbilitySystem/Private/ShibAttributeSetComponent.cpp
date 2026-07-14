// Copyright Shiba Inu Games LLC.

#include "ShibAttributeSetComponent.h"
#include "ShibAttribute.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

// Sets default values for this component's properties
UShibAttributeSetComponent::UShibAttributeSetComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true; // for the initialize component function to be called internally
	bReplicateUsingRegisteredSubObjectList = true;
	SetIsReplicatedByDefault(true);
}

void UShibAttributeSetComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	AActor* Owner = GetOwner();
	if (!Owner) return;

	if (!Owner->HasAuthority()) return;

	for (const auto& Attr : DefaultAttributes)
	{
		AddInstancedAttribute(Attr);
	}
}

void UShibAttributeSetComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	
	if (IsUsingRegisteredSubObjectList())
	{
		for (const auto& Attribute : Attributes)
		{
			if (!Attribute) continue;
			AddReplicatedSubObject(Attribute);
		}
	}
	
	SetSpawnedAttributesListDirty();
}

void UShibAttributeSetComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UShibAttributeSetComponent, Attributes, Params);
}

UShibAttribute* UShibAttributeSetComponent::AddInstancedAttribute(const TSubclassOf<UShibAttribute>& AttributeClass)
{
	auto* NewAttribute = NewObject<UShibAttribute>(this, AttributeClass);
	if (!ensure(NewAttribute)) return nullptr;
	if (Attributes.Find(NewAttribute) != INDEX_NONE) return nullptr;
	NewAttribute->InitAttribute();
	Attributes.Add(NewAttribute);
	return NewAttribute;
}

bool UShibAttributeSetComponent::RemoveInstancedAttribute(UShibAttribute* AttributeToRemove)
{
	return Attributes.RemoveSingle(AttributeToRemove) > 0;
}

void UShibAttributeSetComponent::AddReplicatedAttribute(UShibAttribute* NewAttribute)
{
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		AddReplicatedSubObject(NewAttribute);
		SetSpawnedAttributesListDirty();
	}	
}

void UShibAttributeSetComponent::RemoveReplicatedAttribute(UShibAttribute* AttributeToRemove)
{
	if (IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(AttributeToRemove);
		SetSpawnedAttributesListDirty();
	}
}

void UShibAttributeSetComponent::BeginPlay()
{
	Super::BeginPlay();
}

UShibAttribute* UShibAttributeSetComponent::GetAttribute(FGameplayTag Tag, bool ExactMatch)
{
	if (Attributes.IsEmpty()) return nullptr;

	for (auto Attr : Attributes)
	{
		if (!Attr) continue;
		
		if (ExactMatch)
		{
			if (!Attr->GetGameplayTag().MatchesTagExact(Tag)) continue;
			return Attr;
		}
		
		if (!Attr->GetGameplayTag().MatchesTag(Tag)) continue;
		return Attr;
	}

	return nullptr;
}

const UShibAttribute* UShibAttributeSetComponent::GetAttribute(FGameplayTag Tag, bool ExactMatch) const
{
	if (Attributes.IsEmpty()) return nullptr;
	
	for (auto Attr : Attributes)
	{
		if (!Attr) continue;
		
		if (ExactMatch)
		{
			if (!Attr->GetGameplayTag().MatchesTagExact(Tag)) continue;
			return Attr;
		}
		
		if (!Attr->GetGameplayTag().MatchesTag(Tag)) continue;
		return Attr;
	}

	return nullptr;
}

bool UShibAttributeSetComponent::GetAttributeBase(FGameplayTag Tag, float& OutValue) const
{
	const auto* Attr = GetAttribute(Tag);
	if (Attr == nullptr) return false;
	OutValue = Attr->GetBaseValue();
	return true;
}

bool UShibAttributeSetComponent::GetAttributeCurrent(FGameplayTag Tag, float& OutValue) const
{
	const auto* Attr = GetAttribute(Tag);
	if (Attr == nullptr) return false;
	OutValue = Attr->GetCurrentValue();
	return true;
}

void UShibAttributeSetComponent::AddAttribute(TSubclassOf<UShibAttribute> AttributeClass)
{
	if (!ensure(AttributeClass)) return;

	// For now, we're only dealing with replicated attributes. This is why we want to make sure all attributes are added by the server only
	if (!GetOwner()->HasAuthority()) return;

	if (auto* NewAttribute = AddInstancedAttribute(AttributeClass)) AddReplicatedAttribute(NewAttribute); 
}

void UShibAttributeSetComponent::RemoveAttribute(FGameplayTag Tag)
{
	// For now, we're only dealing with replicated attributes. This is why we want to make sure all attributes are removed by the server only
	if (!GetOwner()->HasAuthority()) return;

	auto* Attr = GetAttribute(Tag);
	if (Attr == nullptr) return;
	RemoveReplicatedAttribute(Attr);
}

bool UShibAttributeSetComponent::AddToCurrentValue(FGameplayTag Tag, float Value, float& NewValue)
{
	//if (!GetOwner()->HasAuthority()) return false;;

	auto* Attr = GetAttribute(Tag);
	if (Attr == nullptr) return false;
	NewValue = Attr->SetCurrentValue(Attr->GetCurrentValue() + Value);
	return true;
}

bool UShibAttributeSetComponent::AddToBaseValue(FGameplayTag Tag, float Value, float& NewValue)
{
	//if (!GetOwner()->HasAuthority()) return false;

	auto* Attr = GetAttribute(Tag);
	if (Attr == nullptr) return false;
	NewValue = Attr->SetBaseValue(Attr->GetBaseValue() + Value);
	return true;
}

bool UShibAttributeSetComponent::SubtractToCurrentValue(FGameplayTag Tag, float Value, float& NewValue)
{
	//if (!GetOwner()->HasAuthority()) return false;;

	auto* Attr = GetAttribute(Tag);
	if (Attr == nullptr) return false;
	NewValue = Attr->SetCurrentValue(Attr->GetCurrentValue() - Value);
	return true;
}

bool UShibAttributeSetComponent::SubtractToBaseValue(FGameplayTag Tag, float Value, float& NewValue)
{
	//if (!GetOwner()->HasAuthority()) return false;

	auto* Attr = GetAttribute(Tag);
	if (Attr == nullptr) return false;
	NewValue = Attr->SetBaseValue(Attr->GetBaseValue() - Value);
	return true;
}

bool UShibAttributeSetComponent::AddModifier(FGameplayTag AttributeTag, TSubclassOf<UShibAttributeModifier> ModifierClass)
{
	if (!GetOwner()->HasAuthority()) return false;

	auto* Attr = GetAttribute(AttributeTag);
	if (Attr == nullptr) return false;
	return Attr->AddModifier(ModifierClass);
}

bool UShibAttributeSetComponent::RemoveModifier(FGameplayTag AttributeTag, FGameplayTag ModifierTag, bool bRemoveAll)
{
	if (!GetOwner()->HasAuthority()) return false;

	auto* Attr = GetAttribute(AttributeTag);
	if (Attr == nullptr) return false;
	return Attr->RemoveModifier(ModifierTag, bRemoveAll);
}

void UShibAttributeSetComponent::OnRep_SpawnedAttributes(const TArray<UShibAttribute*>& PreviousSpawnedAttributes)
{
	if (IsUsingRegisteredSubObjectList())
	{
		// Find the attributes that got removed
		for (UShibAttribute* PreviousAttributeSet : PreviousSpawnedAttributes)
		{
			if (PreviousAttributeSet)
			{
				if (Attributes.Find(PreviousAttributeSet) == INDEX_NONE)
				{
					RemoveReplicatedSubObject(PreviousAttributeSet);
				}
			}
		}

		// Find the attributes that got added
		for (UShibAttribute* NewAttributeSet : Attributes)
		{
			if (IsValid(NewAttributeSet))
			{
				if (PreviousSpawnedAttributes.Find(NewAttributeSet) == INDEX_NONE)
				{
					AddReplicatedSubObject(NewAttributeSet);
				}
			}
		}
	}
}

void UShibAttributeSetComponent::SetSpawnedAttributesListDirty()
{
	MARK_PROPERTY_DIRTY_FROM_NAME(UShibAttributeSetComponent, Attributes, this);
}
