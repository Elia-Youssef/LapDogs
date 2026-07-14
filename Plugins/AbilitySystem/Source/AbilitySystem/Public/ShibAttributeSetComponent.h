// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ShibAttributeSetComponent.generated.h"

class UShibAttribute;
class UShibAttributeModifier;

UCLASS()
class ABILITYSYSTEM_API UShibAttributeSetComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UShibAttributeSetComponent();
	
	virtual void ReadyForReplication() override;

	/** Called to set replicated components */
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	
	/** Initialize component */ 
	virtual void InitializeComponent() override;

	/* Create and add new instanced attribute */
	UShibAttribute* AddInstancedAttribute(const TSubclassOf<UShibAttribute>& AttributeClass);
	
	/* Remove instanced attribute */
	bool RemoveInstancedAttribute(UShibAttribute* AttributeToRemove);
	
	/** Add a new attribute to the replicated subobject list */
	void AddReplicatedAttribute(UShibAttribute* Attribute);

	/** Remove an existing attribute from the replicated subobject list */
	void RemoveReplicatedAttribute(UShibAttribute* Attribute);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	UShibAttribute* GetAttribute(FGameplayTag Tag, bool ExactMatch=true);

	const UShibAttribute* GetAttribute(FGameplayTag Tag, bool ExactMatch=true) const;

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	bool GetAttributeBase(FGameplayTag Tag, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	bool GetAttributeCurrent(FGameplayTag Tag, float& OutValue) const;

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	void AddAttribute(TSubclassOf<UShibAttribute> AttributeClass);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	void RemoveAttribute(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	bool AddToCurrentValue(FGameplayTag Tag, float Value, float& NewValue);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	bool AddToBaseValue(FGameplayTag Tag, float Value, float& NewValue);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	bool SubtractToCurrentValue(FGameplayTag Tag, float Value, float& NewValue);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	bool SubtractToBaseValue(FGameplayTag Tag, float Value, float& NewValue);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TArray<TSubclassOf<UShibAttribute>> DefaultAttributes;

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	bool AddModifier(FGameplayTag AttributeTag, TSubclassOf<UShibAttributeModifier> ModifierClass);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Attribute Set Component")
	bool RemoveModifier(FGameplayTag AttributeTag, FGameplayTag ModifierTag, bool bRemoveAll=true);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, ReplicatedUsing = OnRep_SpawnedAttributes, Transient)
	TArray<UShibAttribute*> Attributes;

	UFUNCTION()
	void OnRep_SpawnedAttributes(const TArray<UShibAttribute*>& PreviousSpawnedAttributes);

private:
	// Needs to be called when modifying the SpawnedAttributes array for changes to be replicated
	void SetSpawnedAttributesListDirty();
};
