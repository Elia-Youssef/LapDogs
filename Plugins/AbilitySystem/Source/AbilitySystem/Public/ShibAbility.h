// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ShibAbilityComponent.h"
#include "UObject/Object.h"
#include "ShibAbility.generated.h"

class AGameState;
class AGameMode;
class UASAbilityComponent;
class UAnimMontage;
class UGameplayTask;
class UGameplayTasksComponent;
class UShibAbilityComponent;

UENUM(BlueprintType)
enum class ENetExecution : uint8
{
	// This ability is initiated, removed and started on server only, but replicates to other clients.
	// If owning client attempts to start the ability, nothing happens locally, but will immediately RPC to the server to attempt to Start.
	// ServerOnly also has the capability to multicast as the ability is replicated.
	NE_ServerOnly UMETA(DisplayName = "Server Only"),
	// This ability is instantiated and removed by the server only but will run predictively on the local client if there is one
	NE_OwnerPredictedAndServer UMETA(DisplayName = "Owner Predicted and Server"),
	// This ability is instantiated and removed by the server only but will also run on the local client if one exists
	NE_ServerAndOwner UMETA(DisplayName = "Server and Owner"),
	// This ability does not replicate and is therefore locally initiated, removed and started. It is useful for internal abilities such as cooldowns or effects created from multicasts.
	NE_NotReplicated UMETA(DisplayName = "Not Replicated (Local Only)"),
};

UCLASS(Blueprintable)
class ABILITYSYSTEM_API UShibAbility : public UObject
{
	GENERATED_BODY()
	
public:

	UShibAbility();
	
	virtual int32 GetFunctionCallspace(UFunction* Function, FFrame* Stack) override;

	virtual bool CallRemoteFunction(UFunction* Function, void* Parameters, FOutParmRec* OutParms, FFrame* Stack) override;
	
	virtual bool IsSupportedForNetworking() const override { return true; }

	/** Called to set replicated components */
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	UFUNCTION(BlueprintNativeEvent, Category = "Shib Ability System|Ability")
	bool CanStart(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Shib Ability System|Ability")
	bool CanStop(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Shib Ability System|Ability")
	bool Start(AActor* Instigator);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Shib Ability System|Ability")
	bool Stop(AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent, Category = "Shib Ability System|Ability")
	bool Cancel(AActor* Instigator);

	/**
	* Called when the ability is added.
	* Depending of the net execution, this could be called on the server and on the owning client too
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void OnAdd(AActor* Instigator);
	
	/**
	* Called when the ability is removed.
	* Depending of the net execution, this could be called on the server and on the owning client too
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void OnRemove();
	
	/**
	* Called when the ability has successfully started.
	* Depending of the net execution, this could be called on the server and on the owning client too
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void OnStart(AActor* Instigator);

	/**
	* This is called on owning Client and Server when the ability has successfully stopped.
	* Depending of the net execution, this could be called on the server and on the owning client too
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void OnStop(AActor* Instigator);

	/**
	* This is called on owning Client and Server when the ability has successfully canceled.
	* Depending of the net execution, this could be called on the server and on the owning client too
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void OnCancel(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	FName GetAbilityName() const { return AbilityName; };

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	FGameplayTagContainer IsBlockedBy(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	bool IsRunning() const { return bRunning; }
	
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	FGameplayTag GetGameplayTag() const { return AbilityTag; }

	UFUNCTION(BlueprintPure)
	ENetExecution GetNetExecution() const { return NetExecution; }

	bool IsNetExecution(ENetExecution NewNetExecution) const { return NewNetExecution==NetExecution; }

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	bool GetOwnerHasAutority() const { return GetOwningComponent()->GetOwner()->HasAuthority(); }

	// Apply cooldown effect if valid
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	void StartCooldown(AActor* Instigator);
	
	// This ability will start automatically immediately after being added 
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Automation")
	bool bAutoStart;

	// This ability will stop automatically immediately after start
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Automation")
	bool bAutoStop;
	
	// This capacity will automatically start its cooldown if there is one
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Automation")
	bool bAutoStartCooldown;

	// // This capacity will be automatically removed immediately after stop.
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Automation")
	bool bAutoRemove;
	
	// The Ability Effect representing the cooldown. If the reference is valid, the effect will be applied automatically when the ability is stopped
	UPROPERTY(EditDefaultsOnly, Category = "Ability|Cooldown")
	TSubclassOf<class UShibAbility> CooldownEffectClass;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FName AbilityName;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTagContainer TagsGranted;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	ENetExecution NetExecution = ENetExecution::NE_ServerOnly;
	
	// Set this to true to sent notifications to the owner of this ability at each state changes (Start, Stop, Cancel, Cooldown)
	// Server ONLY!
	UPROPERTY(EditAnywhere, Category = "Ability|Automation")
	bool bAbilityNotifyOwner;

	// Set this to true to sent notifications to everybody at each state changes (Start, Stop, Cancel, Cooldown)
	// The notification will be sent where the function is executed (Server, owning client and/or other clients).
	UPROPERTY(EditAnywhere, Category = "Ability|Automation")
	bool bAbilityNotifyAll;

	UPROPERTY(Replicated)
	bool bRunning = false;
	
	/**
	 * Client function to call OnStart on the owning client
	 * This is used when the net execution is set to Server and client
	 * The server will call OnStart but it will also call it on the owning client
	 */
	UFUNCTION(Client, Reliable)
	void Client_OnStart(AActor* Instigator);

	/**
	 * Client function to call OnStop on the owning client
	 * This is use when the net execution is set to Server and client
	 * The server will call OnStop but it will also call it on the owning client
	*/
	UFUNCTION(Client, Reliable)
	void Client_OnStop(AActor* Instigator);

	/**
	 * Client function to call OnCancel on the owning client
	 * This is use when the net execution is set to Server and client
	 * The server will call OnStop but it will also call it on the owning client
	*/
	UFUNCTION(Client, Reliable)
	void Client_OnCancel(AActor* Instigator);

	/**
	 * Called when the ability can't start, will notify the owner if owner's notification is enable
	 */
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	void BroadcastAbilityCantStart(const AActor* Instigator);
	
	//We need to redefine the GetWorld to Get the world of the outer
	//object (the actor owning the component)
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	UWorld* GetWorld() const override;

	//Getter Game Mode Base
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	AGameModeBase* GetGameModeBase() const;

	// Getter Game State Base
	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	AGameStateBase* GetGameStateBase() const;

	UFUNCTION(BlueprintCallable, Category = "Shib Ability System|Ability")
	virtual UShibAbilityComponent* GetOwningComponent() const;
};