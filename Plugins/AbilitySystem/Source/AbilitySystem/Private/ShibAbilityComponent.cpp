// Copyright Shiba Inu Games LLC.

#include "ShibAbilityComponent.h"

#include "ShibAbilityEffect.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

UShibAbilityComponent::UShibAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsInitializeComponent = true; // for the initialize component function to be called internally
	bReplicateUsingRegisteredSubObjectList = true;
	SetIsReplicatedByDefault(true);
}

void UShibAbilityComponent::InitializeComponent()
{
	Super::InitializeComponent();
	
	AActor* Owner = GetOwner();
	if (!Owner) return;

	if (Owner->HasAuthority())
	{
		InitAbilityActorInfo(Owner, Owner); // Default init to our outer owner
	}
	
	if(DefaultAbilities.IsEmpty()) return;
	
	// Add default abilities
	for (const auto& Ability : DefaultAbilities)
	{
		AddAbility(Ability, Owner);
	}
}

void UShibAbilityComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	if (IsUsingRegisteredSubObjectList())
	{
		// Add default replicated abilities
		for (const auto& Ability : Abilities)
		{
			if (!Ability) continue;
			AddReplicatedSubObject(Ability);
		}
		MARK_PROPERTY_DIRTY_FROM_NAME(UShibAbilityComponent, Abilities, this);
	}
}

void UShibAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UShibAbilityComponent, OwnerActor, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UShibAbilityComponent, AvatarActor, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UShibAbilityComponent, ActiveGameplayTags, Params);
	DOREPLIFETIME_WITH_PARAMS_FAST(UShibAbilityComponent, Abilities, Params);

	//TODO for bigger games, we need to profile server cpu usage on replicated abilities array
}

void UShibAbilityComponent::UninitializeComponent()
{
	Super::UninitializeComponent();

	AbilityStartedDelegate.RemoveAll(this);
	AbilityStoppedDelegate.RemoveAll(this);
	AbilityCancelledDelegate.RemoveAll(this);
	AbilityCooldownDelegate.RemoveAll(this);
}

void UShibAbilityComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	SetOwnerActor(InOwnerActor);
	SetAvatarActor(InAvatarActor);
}

void UShibAbilityComponent::OnRep_OwningActor()
{
	AActor* LocalOwnerActor = GetOwnerActor();
	AActor* LocalAvatarActor = GetAvatarActor();

	if (LocalOwnerActor != OwnerActor || LocalAvatarActor != AvatarActor)
	{
		if (LocalOwnerActor != nullptr)
		{
			InitAbilityActorInfo(LocalOwnerActor, LocalAvatarActor);
		}
		else
		{
			ClearActorInfo();
		}
	}
}

void UShibAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UShibAbilityComponent::ServerStart_Implementation(FGameplayTag AbilityTag, AActor* Instigator)
{
	StartAbility(AbilityTag, Instigator);
}

void UShibAbilityComponent::ServerStop_Implementation(FGameplayTag AbilityTag, AActor* Instigator)
{
	StopAbility(AbilityTag, Instigator);
}

void UShibAbilityComponent::ServerAddAbility_Implementation(TSubclassOf<UShibAbility> AbilityClass, AActor* Instigator)
{
	AddAbility(AbilityClass, Instigator);
}

void UShibAbilityComponent::ServerRemoveAbility_Implementation(FGameplayTag AbilityTag)
{
	RemoveAbility(AbilityTag);
}

void UShibAbilityComponent::ServerCancelAbility_Implementation(FGameplayTag AbilityTag, AActor* Instigator)
{
	CancelAbility(AbilityTag, Instigator);
}

UShibAbility* UShibAbilityComponent::AddAbility(TSubclassOf<UShibAbility> AbilityClass, AActor* Instigator)
{
	if (!ensure(AbilityClass)) return nullptr;
	const UShibAbility* DefaultAbility = AbilityClass->GetDefaultObject<UShibAbility>();
	
	// System doesn't support stacking multiple abilities (or effects) of the same class.
	// Remove the next if statement below if we want to support this feature.
	// We will need a way to keep track of how much instances of the same ability class we currently have and a way to differentiate them between each other.
	// Good idea to do that: FGameplayTagCountContainer
	UShibAbility* AbilityAlreadyAddedFound = nullptr;
	if (FindAndGetOwnedAbility(DefaultAbility->GetGameplayTag(), true, AbilityAlreadyAddedFound)) return nullptr;
	
	if(!GetOwner()->HasAuthority() && !DefaultAbility->IsNetExecution(ENetExecution::NE_NotReplicated))
	{
		// Abilities that are not NotReplicated are always instantiated and replicated on server only
		ServerAddAbility(AbilityClass, Instigator);
		return nullptr;
	}
	
	auto* NewAbility = NewObject<UShibAbility>(this, AbilityClass);
	if (!ensure(NewAbility)) return nullptr;

	if (DefaultAbility->IsNetExecution(ENetExecution::NE_NotReplicated))
	{
		// Client or server add local ability (not replicated or OwnerPredictedAndServer)
		AddInstancedAbility(NewAbility);
	}
	else if (GetOwner()->HasAuthority())
	{
		// Server adds replicated ability
		AddReplicatedAbility(NewAbility);
	}
	else
	{
		return nullptr;
	}

	NewAbility->OnAdd(Instigator);
	
	if (NewAbility->bAutoStart && NewAbility->CanStart(Instigator))
	{
		NewAbility->Start(Instigator);
	}

	return NewAbility;
}

bool UShibAbilityComponent::RemoveAbility(FGameplayTag AbilityTag)
{
	UShibAbility* AbilityToRemove = nullptr;
	if(!FindAndGetOwnedAbility(AbilityTag, true, AbilityToRemove)) return false; // Return false if we didn't find the ability to remove
	
	if (AbilityToRemove->IsRunning()) return false;

	AbilityToRemove->OnRemove();

	// Abilities that are NotReplicated can be removed by the server and clients
	if(AbilityToRemove->IsNetExecution(ENetExecution::NE_NotReplicated))
	{
		return RemoveInstancedAbility(AbilityToRemove);
	}

	// only server can remove replicated ability
	// If we're here it means the ability is replicated
	if (GetOwner()->HasAuthority()) 
	{
		return RemoveReplicatedAbility(AbilityToRemove);
	}

	return false;
}

bool UShibAbilityComponent::StartAbility(FGameplayTag AbilityTag, AActor* Instigator)
{
	UShibAbility* AbilityToStart = nullptr;
	if(FindAndGetOwnedAbility(AbilityTag, true, AbilityToStart))
	{
		// If the ability found isn't set to not replicated, we need to make sure we're calling it on the proper authoritative actor
		if (!GetOwner()->HasAuthority() && !AbilityToStart->IsNetExecution(ENetExecution::NE_NotReplicated))
		{
			// We first check on the client side if we can actually start the ability.
			// This is just to minimize the amount of rpcs we send to the server, noting bad security wise.
			// If for some reason, the ability can be started on the client, but not on the server, nothing will happen on the server side.
			// Then we ask the server to start the ability as well
			if (!AbilityToStart->CanStart(Instigator)) return false;
			ServerStart(AbilityTag, Instigator);
			
			// For OwnerPredictedAndServer abilities only.
			// Since we're locally controlled here, we want to run the ability predictively before the server.
			// We asked the server start it normally, but we don't wait for it to tell us to run it.
			if (AbilityToStart->IsNetExecution(ENetExecution::NE_OwnerPredictedAndServer))
			{
				return AbilityToStart->Start(Instigator);
			}

			// If this ability isn't owned locally by the client, we stop here.
			return true;
		}
		
		if (!AbilityToStart->CanStart(Instigator)) return false;
		return AbilityToStart->Start(Instigator);
	}
	
	return false;
}

bool UShibAbilityComponent::StopAbility(FGameplayTag AbilityTag, AActor* Instigator)
{
	UShibAbility* AbilityToStop = nullptr;
	if(FindAndGetOwnedAbility(AbilityTag, true, AbilityToStop))
	{
		if (!AbilityToStop->IsRunning()) return false;
		return AbilityToStop->Stop(Instigator);
	}
	
	return false;
}

bool UShibAbilityComponent::CancelAbility(FGameplayTag AbilityTag, AActor* Instigator)
{
	UShibAbility* AbilityToCancel = nullptr;
	
	if(FindAndGetOwnedAbility(AbilityTag, true, AbilityToCancel))
	{
		return AbilityToCancel->Cancel(Instigator);;
	}

	return false;
}

void UShibAbilityComponent::CancelAllAbilities(AActor* Instigator)
{
	TArray<TObjectPtr<UShibAbility>> OwnedAbilities;
	
	// Cancel replicated abilities only on the server
	if (GetOwner()->HasAuthority())
	{
		OwnedAbilities = GetOwnedAbilities(false);

		for (auto  AbilityToCancel : OwnedAbilities)
		{
			AbilityToCancel->Cancel(Instigator);
		}
		return;
	}

	// Cancel local abilities
	OwnedAbilities = GetOwnedAbilities(true);
	for (auto  AbilityToCancel : OwnedAbilities)
	{
		AbilityToCancel->Cancel(Instigator);
	}
}

bool UShibAbilityComponent::AddReplicatedAbility(UShibAbility* NewAbility)
{
	if( ! GetOwner()->HasAuthority()) return false;
	
	if (Abilities.Find(NewAbility) == INDEX_NONE)
	{
		Abilities.Add(NewAbility);
		//Only replicate abilities that are not Server Only
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			AddReplicatedSubObject(NewAbility);
			MARK_PROPERTY_DIRTY_FROM_NAME(UShibAbilityComponent, Abilities, this);
		}
	}
	
	return true;
}

bool UShibAbilityComponent::AddInstancedAbility(UShibAbility* NewAbility)
{
	if (LocalAbilities.Find(NewAbility) == INDEX_NONE)
	{
		LocalAbilities.Add(NewAbility);
	}
	return true;
}

bool UShibAbilityComponent::RemoveInstancedAbility(UShibAbility* AbilityToRemove)
{
	return LocalAbilities.Remove(AbilityToRemove) != 0;
}

bool UShibAbilityComponent::RemoveReplicatedAbility(UShibAbility* AbilityToRemove)
{
	if( ! GetOwner()->HasAuthority()) return false;

	const bool bWasRemoved = Abilities.Remove(AbilityToRemove) != 0;
	
	if (bWasRemoved && IsUsingRegisteredSubObjectList() && IsReadyForReplication())
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(UShibAbilityComponent, Abilities, this);
		RemoveReplicatedSubObject(AbilityToRemove);
	}

	return bWasRemoved;
}

void UShibAbilityComponent::AddReplicatedGameplayTags(const FGameplayTagContainer* Tags)
{
	if( ! GetOwner()->HasAuthority()) return;
	ActiveGameplayTags.AppendTags(*Tags);
	MARK_PROPERTY_DIRTY_FROM_NAME(UShibAbilityComponent, ActiveGameplayTags, this);
}
	
void UShibAbilityComponent::AddLocalGameplayTags(const FGameplayTagContainer* Tags)
{
	LocalActiveGameplayTags.AppendTags(*Tags);
}

void UShibAbilityComponent::RemoveReplicatedGameplayTags(const FGameplayTagContainer* Tags)
{
	if( ! GetOwner()->HasAuthority()) return;
	ActiveGameplayTags.RemoveTags(*Tags);
	MARK_PROPERTY_DIRTY_FROM_NAME(UShibAbilityComponent, ActiveGameplayTags, this);
}

void UShibAbilityComponent::RemoveLocalGameplayTags(const FGameplayTagContainer* Tags)
{
	LocalActiveGameplayTags.RemoveTags(*Tags);
}

void UShibAbilityComponent::SetOwnerActor(AActor* NewOwnerActor)
{
	if (OwnerActor)
	{
		OwnerActor->OnDestroyed.RemoveDynamic(this, &UShibAbilityComponent::OnOwnerActorDestroyed);
	}
	OwnerActor = NewOwnerActor;
	MARK_PROPERTY_DIRTY_FROM_NAME(UShibAbilityComponent, OwnerActor, this);
	if (OwnerActor)
	{
		OwnerActor->OnDestroyed.AddUniqueDynamic(this, &UShibAbilityComponent::OnOwnerActorDestroyed);
	}
}

void UShibAbilityComponent::SetAvatarActor(AActor* NewAvatarActor)
{
	if (AvatarActor)
	{
		AvatarActor->OnDestroyed.RemoveDynamic(this, &UShibAbilityComponent::OnAvatarActorDestroyed);
	}
	AvatarActor = NewAvatarActor;
	MARK_PROPERTY_DIRTY_FROM_NAME(UShibAbilityComponent, AvatarActor, this);
	if (AvatarActor)
	{
		AvatarActor->OnDestroyed.AddUniqueDynamic(this, &UShibAbilityComponent::OnAvatarActorDestroyed);
	}
}

TArray<TObjectPtr<UShibAbility>> UShibAbilityComponent::GetOwnedAbilities(bool LocalAbilitiesOnly)
{
	//return local abilities only
	if (LocalAbilitiesOnly)
	{
		return LocalAbilities;
	}
	
	TArray<TObjectPtr<UShibAbility>> FoundAbilities;
	//Append both local and replicated abilities to the array
	FoundAbilities.Append(Abilities);
	FoundAbilities.Append(LocalAbilities);

	return FoundAbilities;
}

bool UShibAbilityComponent::FindAndGetOwnedAbility(FGameplayTag AbilityTag, bool ExactMatch, UShibAbility*& FoundAbility)
{
	// First we search in the replicated abilities array
	for (TObjectPtr<UShibAbility>& Ability : Abilities)
	{
		if (!Ability) continue;
		if (ExactMatch)
		{
			if (!Ability->GetGameplayTag().MatchesTagExact(AbilityTag)) continue;
			FoundAbility = Ability;
			return true;
		}
		if (!Ability->GetGameplayTag().MatchesTag(AbilityTag)) continue;
		FoundAbility = Ability;
		return true;
	}

	// If we didn't find it at this point, we search in the non replicated abilities array
	for (TObjectPtr<UShibAbility>& Ability : LocalAbilities)
	{
		if (!Ability) continue;
		if (ExactMatch)
		{
			if (!Ability->GetGameplayTag().MatchesTagExact(AbilityTag)) continue;
			FoundAbility = Ability;
			return true;
		}
		if (!Ability->GetGameplayTag().MatchesTag(AbilityTag)) continue;
		FoundAbility = Ability;
		return true;
	}
	return false;
}

void UShibAbilityComponent::OnAvatarActorDestroyed(AActor* InActor)
{
	if (InActor == AvatarActor)
	{
		AvatarActor = nullptr;
		MARK_PROPERTY_DIRTY_FROM_NAME(UShibAbilityComponent, AvatarActor, this);
	}
}

void UShibAbilityComponent::OnOwnerActorDestroyed(AActor* InActor)
{
	if (InActor == OwnerActor)
	{
		OwnerActor = nullptr;
		MARK_PROPERTY_DIRTY_FROM_NAME(UShibAbilityComponent, OwnerActor, this);
	}
}

void UShibAbilityComponent::ClearActorInfo()
{
	SetOwnerActor(nullptr);
	SetAvatarActor(nullptr);
}

void UShibAbilityComponent::Client_BroadcastAbilityCantStart_Implementation(const FGameplayTag Tag, const AActor* Owner,
                                                                            const AActor* Instigator)
{
	AbilityCantStartDelegate.Broadcast(Tag, Owner, Instigator);
}

void UShibAbilityComponent::Client_AbilityCanStartAcknowledge_Implementation(const FGameplayTag Tag, const bool bCanStart, const bool bIsRunning, const AActor* Owner,
                                                                             const AActor* Instigator)
{
	// For owner predicted abilities.
	// After starting an owner predicted ability, we double check on the client
	// to ensure the ability is synchronized with the server.
	UShibAbility* AbilityFound = nullptr;
	if (FindAndGetOwnedAbility(Tag,true,AbilityFound))
	{
		bool AbilityIsRunning = AbilityFound->IsRunning();
		if (bCanStart)
		{
			if (AbilityIsRunning != bIsRunning && !AbilityIsRunning)
			{
				AbilityFound->Start(GetOwnerActor()); // Ability is active on the server, but not on the client. We start it right away!
			}
		}
		else
		{
			if (AbilityIsRunning != bIsRunning && AbilityIsRunning) // If true, not sync with the server and shouldn't be active!
			{
				AbilityFound->Cancel(GetOwnerActor());
			}
		}
	}
}

void UShibAbilityComponent::Client_BroadcastAbilityStarted_Implementation(const FGameplayTag Tag, const AActor* Owner, const AActor* Instigator, const float ExecutionTime)
{
	AbilityStartedDelegate.Broadcast(Tag, Owner, Instigator, ExecutionTime);
}

void UShibAbilityComponent::Client_BroadcastAbilityStopped_Implementation(const FGameplayTag Tag, const AActor* Owner, const AActor* Instigator)
{
	AbilityStoppedDelegate.Broadcast(Tag, Owner, Instigator);
}

void UShibAbilityComponent::Client_BroadcastAbilityCancelled_Implementation(const FGameplayTag Tag, const AActor* Owner, const AActor* Instigator)
{
	AbilityCancelledDelegate.Broadcast(Tag, Owner, Instigator);
}

void UShibAbilityComponent::Client_BroadcastAbilityCooldown_Implementation(const FGameplayTag Tag, const float CooldownDuration)
{
	AbilityCooldownDelegate.Broadcast(Tag, CooldownDuration);
}
