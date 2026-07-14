// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ShibAbilityComponent.generated.h"

class UShibAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusNotify, const FGameplayTag, Tag, const AActor*, Owner, const AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FAbilityStatusNotifyWithExecutionTime, const FGameplayTag, Tag, const AActor*, Owner, const AActor*, Instigator, float, ExecutionTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAbilityCooldown, const FGameplayTag, Tag, const float, CooldownDuration);

UCLASS()
class ABILITYSYSTEM_API UShibAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** Sets default values for this component's properties */ 
	UShibAbilityComponent();
	
	/** Initialize component */ 
	virtual void InitializeComponent() override;
	
	virtual void ReadyForReplication() override;
	
	/** Called to set replicated components */
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	virtual void UninitializeComponent() override;
	
	/** Set the current owner and avatar actor representing this component */ 
	UFUNCTION()
	void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor);

	/** When Owner or avatar actor change */ 
	UFUNCTION()
	void OnRep_OwningActor();

	/**
	* Create and add a new ability
	* The system currently doesn't support stacking multiple abilities of the same class. So if we find an ability instance of the same class as the one we want to add,
	* we don't create and add a new one.
	*/
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability Component",  meta = (ReturnDisplayName = "New Ability"))
	virtual UShibAbility* AddAbility(TSubclassOf<UShibAbility> AbilityClass, AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability Component")
	virtual bool RemoveAbility(FGameplayTag AbilityTag);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability Component")
	virtual bool StartAbility(FGameplayTag AbilityTag, AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability Component")
	virtual bool StopAbility(FGameplayTag AbilityTag, AActor* Instigator);

	/**
	* Stop the execution of an ability.
	* This doesn't remove it.
	*/ 
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability Component")
	virtual bool CancelAbility(FGameplayTag AbilityTag, AActor* Instigator);

	/**
	* Cancel all the active abilities, this stop the execution of all the abilities
	* Must be called by the server
	*/
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability Component")
	void CancelAllAbilities(AActor* Instigator);
	
	/** Add a replicated gameplay ability associated to this component */
	bool AddReplicatedAbility(UShibAbility* NewAbility);

	/** Add a local gameplay ability associated to this component */
	bool AddInstancedAbility(UShibAbility* NewAbility);

	/** Remove a replicated gameplay ability associated to this component */
	bool RemoveReplicatedAbility(UShibAbility* AbilityToRemove);
	
	/** Remove a local gameplay ability associated to this component */
	bool RemoveInstancedAbility(UShibAbility* AbilityToRemove);

	/** Add a replicated gameplay Tags */
	void AddReplicatedGameplayTags(const FGameplayTagContainer* Tags);

	/** Add a local gameplay Tags */
	void AddLocalGameplayTags(const FGameplayTagContainer* Tags);

	/** Remove a replicated gameplay Tags */
	void RemoveReplicatedGameplayTags(const FGameplayTagContainer* Tags);

	/** Remove a local gameplay Tags */
	void RemoveLocalGameplayTags(const FGameplayTagContainer* Tags);
	
	/** Change the Owner actor */
	void SetOwnerActor(AActor* NewOwnerActor);

	/** Changes the avatar actor, leaves the owner actor the same */
	void SetAvatarActor(AActor* NewAvatarActor);

	/**
	* Return the current active abilities.
	* Append both local and replicated array, return all the owned abilities.
	* We need to pay attention to where this function is called.
	* The returned items in the array will depend from where we're calling this function since the local abilities array isn't replicated.
	*/
	TArray<TObjectPtr<UShibAbility>> GetOwnedAbilities(bool LocalAbilitiesOnly=false);

	/**
	* Search and return the owned ability based on the provided Gameplay Tag.
	* We're looking in both, replicated and non replicated array to try to find the ability.
	* If exact match is false, it will return the first ability found with matching tags.
	* Return true paired with the ability reference when the ability is found.
	*/
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability Component")
	bool FindAndGetOwnedAbility(FGameplayTag AbilityTag, bool ExactMatch, UShibAbility*& FoundAbility);

	/* Return the current owner of this component */
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability Component")
	AActor* GetOwnerActor() const { return OwnerActor; }

	/* Return the current avatar actor that is using this component */
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability Component")
	/** Returns the avatar actor for this component */
	AActor* GetAvatarActor() const { return AvatarActor; };

	/** Function called when the avatar actor gets destroyed */
	UFUNCTION()
	void OnAvatarActorDestroyed(AActor* InActor);

	/** Function called when the owner actor gets destroyed */
	UFUNCTION()
	void OnOwnerActorDestroyed(AActor* InActor);

	/**
	* This is called when the actor that is initialized to this system dies, this will clear that actor from this system
	*/
	virtual void ClearActorInfo();
	
	UFUNCTION(Client, Reliable, BlueprintCallable)
	void Client_AbilityCanStartAcknowledge(const FGameplayTag Tag, const bool bCanStart, const bool bIsRunning, const AActor* Owner, const AActor* Instigator);
	
	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Shib Ability System|Ability Component")
	void Client_BroadcastAbilityCantStart(const FGameplayTag Tag, const AActor* Owner, const AActor* Instigator);

	UFUNCTION(Client, Reliable)
	void Client_BroadcastAbilityStarted(const FGameplayTag Tag, const AActor* Owner, const AActor* Instigator, const float ExecutionTime);

	UFUNCTION(Client, Reliable)
	void Client_BroadcastAbilityStopped(const FGameplayTag Tag, const AActor* Owner, const AActor* Instigator);

	UFUNCTION(Client, Reliable)
	void Client_BroadcastAbilityCancelled(const FGameplayTag Tag, const AActor* Owner, const AActor* Instigator);

	UFUNCTION(Client, Reliable)
	void Client_BroadcastAbilityCooldown(const FGameplayTag Tag, const float CooldownDuration);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Shib Ability System|Ability Component|Tags")
	FGameplayTagContainer ActiveGameplayTags;

	UPROPERTY(BlueprintReadOnly, Category = "Shib Ability System|Ability Component|Tags")
	FGameplayTagContainer LocalActiveGameplayTags;
	
	UPROPERTY(BlueprintAssignable)
	FAbilityStatusNotify AbilityCantStartDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FAbilityStatusNotifyWithExecutionTime AbilityStartedDelegate;

	UPROPERTY(BlueprintAssignable)
	FAbilityStatusNotify AbilityStoppedDelegate;

	UPROPERTY(BlueprintAssignable)
	FAbilityStatusNotify AbilityCancelledDelegate;

	UPROPERTY(BlueprintAssignable)
	FAbilityCooldown AbilityCooldownDelegate;

protected:
	//Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerStart(FGameplayTag AbilityTag, AActor* Instigator);

	UFUNCTION(Server, Reliable)
	void ServerStop(FGameplayTag AbilityTag, AActor* Instigator);

	UFUNCTION(Server, Reliable)
	void ServerAddAbility(TSubclassOf<UShibAbility> AbilityClass, AActor* Instigator);

	UFUNCTION(Server, Reliable)
	void ServerRemoveAbility(FGameplayTag AbilityTag);

	UFUNCTION(Server, Reliable)
	void ServerCancelAbility(FGameplayTag AbilityTag, AActor* Instigator);
	
	//Here we are tagging with UPROPERTY since this allow Unreal
	//to track this object and its subobjects
	//in the garbage collector
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Shib Ability System|Ability Component")
	TArray<TObjectPtr<UShibAbility>> Abilities;

	UPROPERTY(BlueprintReadOnly, Category = "Shib Ability System|Ability Component")
	TArray<TObjectPtr<UShibAbility>> LocalAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability System")
	TArray<TSubclassOf<UShibAbility>> DefaultAbilities;

private:

	/** The actor that owns this component logically */
	UPROPERTY(ReplicatedUsing = OnRep_OwningActor)
	TObjectPtr<AActor> OwnerActor;

	/** The actor that is the physical representation used for abilities. Can be NULL */
	UPROPERTY(ReplicatedUsing = OnRep_OwningActor)
	TObjectPtr<AActor> AvatarActor;
};