// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBasePlayerState.h"
#include "ShibPlayerState.generated.h"

class UShibAbilityComponent;
class UShibAttributeSetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerRaceStatusNotifyDelegate, int32, CurrentLap);

/**
 * 
 */
UCLASS()
class SHIBRUN_API AShibPlayerState : public AShibBasePlayerState
{
	GENERATED_BODY()

public:
	AShibPlayerState();

	virtual void BeginPlay() override;

	// Setup custom replicated variables
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/**
	* Function to initialize the ability system for the possessed shib class
	* Apply ability upgrades
	* This function should be called by the character once it is spawned
	* It must be called from the server
	*/
	bool InitializeAbilitySystemForShibClass();

	/**
	* Function to Uninitialize the ability system for the possessed shib class
	* Stop ability upgrades
	* This function should be called by the character before gettiing destroyed
	* It must be called from the server
	*/
	void UninitializeAbilitySystemForShibClass();
	
	/**
	* Increase the lap counter when a player cross the finish line
	* Only the server can use this function
	*/
	UFUNCTION(BlueprintCallable, Category = "ShibPlayerState|Gameplay")
	void IncreaseLapCounter();

	/**
	* Return the current lap of this player
	*/
	UFUNCTION(BlueprintGetter, Category = "ShibPlayerState|Gameplay")
	int32 GetCurrentLap(){ return CurrentLap; }

	/**
	* Set bPlayerIsReadyToRace to true and notify the game mode
	* SERVER ONLY
	*/
	UFUNCTION(BlueprintCallable, Category = "ShibPlayerState|Gameplay")
	void SetPlayerIsReadyToRace();

	/**
	* Get bPlayerIsReadyToRace
	* If true, the player is fully initialized and ready to race
	*/
	UFUNCTION(BlueprintCallable, Category = "ShibPlayerState|Gameplay")
	bool GetPlayerIsReadyToRace() { return bPlayerIsReadyToRace; }
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability System")
	bool bIsAbilitySystemInitialized;

	// Ability component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Ability System")
	TObjectPtr<UShibAbilityComponent> Ability;

	// Attribute Set component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Ability System")
	TObjectPtr<UShibAttributeSetComponent> Attributes;

	// Lap counter for the player
	UPROPERTY(ReplicatedUsing=OnRep_CurrentLap, BlueprintGetter=GetCurrentLap)
	int32 CurrentLap = 0;
	
	UFUNCTION()
	void OnRep_CurrentLap();

	// Total travelled distance for the current lap
	// This distance is calculated using the Route spline actor length placed in the race map and the value is set by the Game State (server only)
	// This value can be used to track down the current player position more precisely than the player position, and it can be used to know where to respawn the player
	UPROPERTY(BlueprintReadOnly, Category = "ShibPlayerState|Gameplay")
	float ConfirmedLapTravelledDistance = 1.f;

	// Delegate used to notify the current player status, which includes the current number of laps and the player position
	// Mainly used by the HUD to display this information
	UPROPERTY(BlueprintAssignable)
	FPlayerRaceStatusNotifyDelegate PlayerRaceStatusNotify;
	
protected:
	/**
	* Copy the player's info from the original player state to the newly create one after a server travel. 
	* This is called by default by the engine once the player moved to another map using the same player state class. 
	* Other default values are also copied in the parent's function, see parent class function 
	*/ 
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
	/**
	* Load the ability upgrades saved in the Game Instance
	*/
	UFUNCTION(Client, Reliable)
	void Client_LoadSavedAbilityUpgrades();
	
	UPROPERTY(BlueprintReadOnly)
	bool bPlayerIsReadyToRace=false;
};
