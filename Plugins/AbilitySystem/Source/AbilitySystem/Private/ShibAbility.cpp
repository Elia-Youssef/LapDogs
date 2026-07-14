// Copyright Shiba Inu Games LLC.

#include "ShibAbility.h"
#include "ShibAbilityComponent.h"
#include "ShibAbilityEffect.h"
#include "Net/UnrealNetwork.h"

UShibAbility::UShibAbility()
{
	bAutoStart = false;
	bAutoStop = false;
	bAutoStartCooldown = true;
	bAutoRemove = false;
	NetExecution = ENetExecution::NE_ServerOnly;
	bAbilityNotifyOwner = false;
	bAbilityNotifyAll = false;
}

int32 UShibAbility::GetFunctionCallspace(UFunction* Function, FFrame* Stack)
{
	if(HasAnyFlags(RF_ClassDefaultObject) || !IsSupportedForNetworking())
	{
		return GEngine->GetGlobalFunctionCallspace(Function, this, Stack);
	}

	check(GetOuter() != nullptr);
	return GetOuter()->GetFunctionCallspace(Function, Stack);
}

bool UShibAbility::CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack)
{
	check(!HasAnyFlags(RF_ClassDefaultObject));
	check(GetOuter()->GetOuter() != nullptr);

	// The reason we call GetOuter twice is to first get the ability component and then get the character
	AActor* Owner = CastChecked<AActor>(GetOuter()->GetOuter());

	bool bProcessed = false;

	FWorldContext* const Context = GEngine->GetWorldContextFromWorld(GetWorld());
	if(Context != nullptr)
	{
		for(FNamedNetDriver& Driver : Context->ActiveNetDrivers)
		{
			if(Driver.NetDriver != nullptr && Driver.NetDriver->ShouldReplicateFunction(Owner, Function))
			{
				Driver.NetDriver->ProcessRemoteFunction(Owner, Function, Parameters, OutParms, Stack, this);
			}
		}
	}
	return bProcessed;
}

void UShibAbility::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UShibAbility, bRunning);
}

bool UShibAbility::CanStart_Implementation(AActor* Instigator)
{
	auto* Comp = GetOwningComponent();
	if (!ensure(Comp)) return false;

	if (bRunning)
	{
		BroadcastAbilityCantStart(Instigator);
		return false;
	}
	
	const bool bBlocked = Comp->ActiveGameplayTags.HasAny(BlockedTags) || Comp->LocalActiveGameplayTags.HasAny(BlockedTags);
	//UE_LOG(LogTemp, Log, TEXT("Can Start %s : %d"), *GetNameSafe(this), !bBlocked);
	
	if (bBlocked)
	{
		BroadcastAbilityCantStart(Instigator);
	}
	
	return !bBlocked;
}

bool UShibAbility::CanStop_Implementation(AActor* Instigator)
{
	return true;
}

bool UShibAbility::Start_Implementation(AActor* Instigator)
{
	//UE_LOG(LogTemp, Log, TEXT("Starting: %s"), *GetNameSafe(this));
	auto* Comp = GetOwningComponent();
	if (!ensure(Comp)) return false;

	bRunning = true;
	
	// Add active tags in the right array, replicated or non replicated based on the net execution
	if (NetExecution == ENetExecution::NE_NotReplicated)
		Comp->AddLocalGameplayTags(&TagsGranted);
	else
		Comp->AddReplicatedGameplayTags(&TagsGranted);

	if (NetExecution == ENetExecution::NE_ServerAndOwner)
	{
		Client_OnStart(Instigator);
	}
	
	OnStart(Instigator);

	if (bAbilityNotifyOwner && GetOwnerHasAutority())
	{
		GetOwningComponent()->Client_BroadcastAbilityStarted(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator, 0.f);
	}

	if (bAbilityNotifyAll)
	{
		GetOwningComponent()->AbilityStartedDelegate.Broadcast(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator, 0.f);
	}

	if (bAutoStop)
	{
		return Stop(Instigator);
	}
	
	return true;
}

void UShibAbility::Client_OnStart_Implementation(AActor* Instigator)
{
	OnStart(Instigator);
}

bool UShibAbility::Stop_Implementation(AActor* Instigator)
{
	//UE_LOG(LogTemp, Log, TEXT("Stopping: %s"), *GetNameSafe(this));
	ensureAlways(bRunning);
	auto* Comp = GetOwningComponent();
	if (!ensure(Comp)) return false;

	if (NetExecution == ENetExecution::NE_ServerAndOwner)
	{
		Client_OnStop(Instigator);
	}
	
	OnStop(Instigator);
	
	// Remove active tags from the right array, replicated or non replicated based on the net execution
	if (NetExecution == ENetExecution::NE_NotReplicated)
		Comp->RemoveLocalGameplayTags(&TagsGranted);
	else
		Comp->RemoveReplicatedGameplayTags(&TagsGranted);	

	bRunning = false;
	
	if (bAbilityNotifyOwner && GetOwnerHasAutority())
	{
		GetOwningComponent()->Client_BroadcastAbilityStopped(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator);
	}

	if (bAbilityNotifyAll)
	{
		GetOwningComponent()->AbilityStoppedDelegate.Broadcast(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator);
	}
	
	// We don't want to start any cooldown if auto remove is true
	if (bAutoRemove)
	{
		// Auto remove the ability after its execution if desired
		return Comp->RemoveAbility(AbilityTag);
	}
	
	// Make sure we have a valid cooldown effect and auto Start cooldown is true to start the cooldown
	// We don't want to start cooldown on client
	if (GetOwnerHasAutority() && IsValid(CooldownEffectClass) && bAutoStartCooldown) 
	{ 
		StartCooldown(Comp->GetOwnerActor());
	}
	
	return true;
}

void UShibAbility::Client_OnStop_Implementation(AActor* Instigator)
{
	OnStop(Instigator);
}

bool UShibAbility::Cancel_Implementation(AActor* Instigator)
{
	auto* Comp = GetOwningComponent();
	if (!ensure(Comp)) return false;
	
	// if the ability isn't running, no need to cancel it.
	if (!bRunning) return false;
		
	//UE_LOG(LogTemp, Log, TEXT("Cancelling: %s"), *GetNameSafe(this));
	
	if (NetExecution == ENetExecution::NE_ServerAndOwner || NetExecution == ENetExecution::NE_OwnerPredictedAndServer)
	{
		Client_OnCancel(Instigator);
	}
    
	OnCancel(Instigator);
	
	// Remove active tags from the right array, replicated or non replicated based on the net execution
	if (NetExecution == ENetExecution::NE_NotReplicated)
		Comp->RemoveLocalGameplayTags(&TagsGranted);
	else
		Comp->RemoveReplicatedGameplayTags(&TagsGranted);

	bRunning = false;
	
	if (bAbilityNotifyOwner && GetOwnerHasAutority())
	{
		GetOwningComponent()->Client_BroadcastAbilityCancelled(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator);
	}

	if (bAbilityNotifyAll)
	{
		GetOwningComponent()->AbilityCancelledDelegate.Broadcast(AbilityTag, GetOwningComponent()->GetOwnerActor(), Instigator);
	}
	
	// We don't want to start any cooldown if auto remove is true
	if (bAutoRemove)
	{
		// Auto remove the ability after its execution if desired
		return Comp->RemoveAbility(AbilityTag);
	}
	
	// Make sure we have a valid cooldown effect and auto Start cooldown is true to start the cooldown
	// We don't want to start cooldown on client
	if (GetOwnerHasAutority() && IsValid(CooldownEffectClass) && bAutoStartCooldown) 
	{ 
		StartCooldown(Comp->GetOwnerActor());
	}
	
	return true;
}

void UShibAbility::Client_OnCancel_Implementation(AActor* Instigator)
{
	OnCancel(Instigator);
}

FGameplayTagContainer UShibAbility::IsBlockedBy(AActor* Instigator)
{
	FGameplayTagContainer BlockedByContainer{};
	if (CanStart(Instigator)) return BlockedByContainer;
	auto* Comp = GetOwningComponent();
	if (!ensure(Comp)) return BlockedByContainer;
	BlockedByContainer.AppendTags(Comp->ActiveGameplayTags.Filter(BlockedTags));
	BlockedByContainer.AppendTags(Comp->LocalActiveGameplayTags.Filter(BlockedTags));
	return BlockedByContainer;
}

void UShibAbility::BroadcastAbilityCantStart(const AActor* Instigator)
{
	if (!bAbilityNotifyOwner) return;
	auto* Comp = GetOwningComponent();
	bool IsAuthority = GetOwnerHasAutority();
	
	APawn* OwningPawn = Cast<APawn>(GetOwningComponent()->GetOwner());
	if (OwningPawn)
	{
		if (!IsAuthority || (IsAuthority && OwningPawn->IsLocallyControlled())) // If we're on the client, or locally controlled on the server
		{
			Comp->AbilityCantStartDelegate.Broadcast(AbilityTag, OwningPawn, Instigator);
		}
		else if (IsAuthority && NetExecution == ENetExecution::NE_ServerOnly) // If we're here, it means the client thinks the ability can be start, but it can't so we let him know
		{
			Comp->Client_BroadcastAbilityCantStart(AbilityTag, OwningPawn, Instigator);
		}
	}
}

UWorld* UShibAbility::GetWorld() const
{
	return GetOuter()->GetWorld();
}

AGameModeBase* UShibAbility::GetGameModeBase() const
{
	return GetWorld()->GetAuthGameMode();
}

AGameStateBase* UShibAbility::GetGameStateBase() const
{
	return GetWorld()->GetGameState();
}

void UShibAbility::StartCooldown(AActor* Instigator)
{
	if (CooldownEffectClass)
	{
		auto* Comp = GetOwningComponent();
		if (!ensure(Comp)) return;

		// Safety check to make sure the ability isn't running
		if (bRunning)
		{
			Cancel(Instigator);
			return;
		}
		
		UShibAbility* AddedCooldownAbility = Comp->AddAbility(CooldownEffectClass, Instigator);
		
		if (AddedCooldownAbility && GetOwnerHasAutority())
		{
			if (UShibAbilityEffect* CooldownAbilityEffect = Cast<UShibAbilityEffect>(AddedCooldownAbility))
			{
				float CooldownDuration;
				if (CooldownAbilityEffect->DurationType.CalculateDuration(GetOwningComponent()->GetOwner(), CooldownDuration))
				{
					Comp->Client_BroadcastAbilityCooldown(AbilityTag, CooldownDuration);
				}
			}
		}
	}
}

UShibAbilityComponent* UShibAbility::GetOwningComponent() const
{
	return Cast<UShibAbilityComponent>(GetOuter());
}