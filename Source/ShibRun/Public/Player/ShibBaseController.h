// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Game/ShibGameInstance.h"
#include "GameFramework/PlayerController.h"
#include "ShibBaseController.generated.h"

class UShibBaseNavigation;
class AShibBasePlayerState;
class AShibBaseGameMode;

UCLASS()
class SHIBRUN_API AShibBaseController : public APlayerController
{
	GENERATED_BODY()

public:
	
	// Called by the game mode after the player join a session, or after a seamless travel
	// This is called from the server
	UFUNCTION()
	virtual void InitializeShibController();
	
	// Create widgets locally
	UFUNCTION(Client, Reliable)
	virtual void Client_CreateHud();
	
	/**
	 * Notify client they were kicked from the server
	 * Base class kick function
	 * Both kick function call the same internal function
	 */
	void ClientWasKicked(const FText& KickReason);

	/**
	 * Custom kick function used in our system
	 * Notify player that he was kicked out of the session
	*/
	UFUNCTION(Client, Reliable)
	void Client_WasKickedFromSession(const FText& KickReason);
	
	/**
	* We override this function to know when the player leaves the game and ask the game mode to unregister the player before the net connection is cleared.
	*/
	virtual void OnNetCleanup(UNetConnection* Connection) override;
	
	// Casts
	void CallCasts();
	
	// Game
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UShibGameInstance> ShibGI;
	TObjectPtr<UShibGameInstance> GetShibGI();

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AShibBasePlayerState> ShibBasePS;
	TObjectPtr<AShibBasePlayerState> GetShibBasePS();

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AShibBaseGameMode> ShibBaseGM;
	TObjectPtr<AShibBaseGameMode> GetShibBaseGM();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> PlayerUI_Class;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UShibBaseNavigation> PlayerUI=nullptr;

protected:
	virtual void BeginPlay() override;

	// Give a chance to blueprint to also initialize stuff for the hud.
	// This function is called after the hud has been created internally by this class.
	UFUNCTION(BlueprintImplementableEvent)
	void InitializeHud();
	
	UFUNCTION(BlueprintImplementableEvent)
	void InternalClientWasKickedFromSession(const FText& KickReason);
};
