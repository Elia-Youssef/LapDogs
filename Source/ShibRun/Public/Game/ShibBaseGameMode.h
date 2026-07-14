// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Player/ShibAiController.h"
#include "Player/ShibBasePlayerState.h"
#include "Utils/ShibTypes.h"
#include "ShibBaseGameMode.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogShibGameMode, Log, All);

enum class EShibClass : uint8;
class AShibRaceManager;
class AShibBaseController;
class UShibGameInstance;
class APlayerStart;

namespace MatchState
{
	const FName Countdown = FName(TEXT("Countdown")); //  Players are fully loaded onto the map and the race countdown can begin.
	const FName EndPending = FName(TEXT("EndPending")); // When the first player crosses the finish line
	const FName PlayerFinishedRace = FName(TEXT("PlayerFinishedRace")); // LOCAL MATCH STATE, DO NOT USE THIS AS A REAL MATCH STATE. Use to notify a specific player that he finishes the race.
}

/**
 * 
 */
UCLASS()
class SHIBRUN_API AShibBaseGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY()
	TObjectPtr<UShibGameInstance> ShibGI;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AShibBaseController> ShibCtrl;
	
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<APlayerController> SpectatorController;
	
	UPROPERTY(BlueprintReadWrite)
	TArray<APlayerController*> ConnectedControllers;

	virtual void PreInitializeComponents() override;
	
	// For adding custom actors to the list of actors that persist between levels
	virtual void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;
	
	/**
	* Function to unregister a player before he leaves the session. We call this function only if the player is legitimately leaving the game or match (E.g. click on quit game).
	* If this function is not called, the game mode will eventually do the cleaning itself, but not directly after the player has left to give the player some time if he wants to join back the session.
	*/
	UFUNCTION(BlueprintCallable)
	virtual void PreLogout(APlayerController* InPlayerController);

	// Kick all players from the game
	UFUNCTION(BlueprintCallable)
	void KickPlayer(APlayerController* Controller, FText KickReason) const;
	
	// Kick all players from the game
	UFUNCTION(BlueprintCallable)
	void KickPlayers(FText KickReason) const;

	UFUNCTION(BlueprintCallable)
	void TravelToNextLevel();

	// Loop through connected controllers and enable/disable movement
	// This function also take care of turning all AI brains on or off
	void EnableShibCharacters(const bool bEnabled, const bool bIgnoreAi);
	
	virtual void RestartPlayer(AController* NewPlayer) override;
	
	//UFUNCTION(BlueprintCallable)
	//virtual void RespawnShibCharacter(AController* Controller = nullptr);

	UFUNCTION(BlueprintCallable)
	bool IsControllerPlayable(AController* Controller);
	
	TObjectPtr<AShibBaseController> GetShibCtrl();
	TObjectPtr<UShibGameInstance> GetShibGI();
	
	bool CanRegisterPlayer(APlayerController* Exiting);
	
protected:
	virtual void BeginPlay() override;
	virtual void Logout(AController* Exiting) override;
	virtual void SwapPlayerControllers(APlayerController* OldPC, APlayerController* NewPC) override;

	//Registering players for new connections via OnPostLogin
	virtual void OnPostLogin(AController* NewPlayer) override;

	// Reinitialize controllers
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	
	// Find a player start
	// If we already own one, look for the player start with our owned tag
	// if we don't own one, get the first available player start and own it
	virtual bool GetAvailablePlayerStart(const FName CheckpointTag, APlayerStart*& PlayerStartFound);

	// Add a owner to a player start
	virtual void SetPlayerStartOwner(APlayerStart* Start, AShibBasePlayerState* PsOwner);
	
	// If a player own a player start, release it. This way, another player can now own it
	virtual void RemovePlayerStartOwner(AShibBasePlayerState* PS);
	
	TSubclassOf<AShibCharacter> GetShibClassToSpawn(EShibClass SelectedClass);

	// Clear all game timers here
	// Called on server travel
	virtual void ClearGameTimers();
	
	// Race manager singleton actor that persist between levels to keep track of the races data
	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<AShibRaceManager> RaceManager;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shib Class")
	TSubclassOf<APawn> SpectatorClassRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shib Class")
	TSubclassOf<AShibCharacter> GoodBoiClassRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shib Class")
	TSubclassOf<AShibCharacter> ZoomyBoiClassRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shib Class")
	TSubclassOf<AShibCharacter> ChonkyBoiClassRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Shib Class")
	TSubclassOf<AShibCharacter> CoolBoiClassRef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Race Manager")
	TSubclassOf<AShibRaceManager> RaceManagerClass;
	
	// Count of all the players we spawned in the map that are waiting for the race to start
	int32 NumPlayersInitializedAndReady=0;

private:
	
	/**
	 * Update all players starting position
	 * This uses the latest race leaderboard to set where players start in the next race
	 * We want the first player to start last, and the last player to start first in the next race
	 * @param bUpdateAiControllers Update Ai controllers or Real player controllers
	 */
	void UpdatePlayerStartsByRank(const bool bUpdateAiControllers=false);
	
#pragma region RandomNames
	// It's not like we have more than 1 game mode, but I decided to make everything static anyway just in case
	static TSet<FString> UsedFirstNames;
	static TSet<FString> UsedLastNames;
	
	FString GetRandomName();
   
	// First name pool
	TArray<FString> FirstNames = {
		TEXT("Menacing"),
		TEXT("Dark"),
		TEXT("Furious"),
		TEXT("Hyper"),
		TEXT("Roaring"),
		TEXT("Speedy"),
		TEXT("Rolling"),
		TEXT("Cool"),
		TEXT("Hungry"),
		TEXT("Stoic"),
		TEXT("Cwazy"),
		TEXT("Eager"),
		TEXT("Howling"),
		TEXT("Rushing"),
		TEXT("Super"),
		TEXT("Rough"),
		TEXT("Epic"),
		TEXT("Unleashed"),
		TEXT("Cameron"),
		TEXT("Dawa"),
		TEXT("Shy")
	};

	// Last name pool
	TArray<FString> LastNames = {
		TEXT("Paws"),
		TEXT("Fang"),
		TEXT("TailSwiper"),
		TEXT("NomNoms"),
		TEXT("RollyPolly"),
		TEXT("Chomper"),
		TEXT("Tippytaps"),
		TEXT("Barker"),
		TEXT("Boops"),
		TEXT("Chaser"),
		TEXT("Chewer"),
		TEXT("Sniffer"),
		TEXT("Swagger"),
		TEXT("Runner"),
		TEXT("Sprinter"),
		TEXT("Tongue"),
		TEXT("Nose"),
		TEXT("Drifter"),
		TEXT("Daton "),
		TEXT("Chan"),
		TEXT("Tuah")
	};

#pragma endregion RandomNames
};


