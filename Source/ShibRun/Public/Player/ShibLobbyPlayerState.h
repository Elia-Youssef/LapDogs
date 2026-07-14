// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBasePlayerState.h"
#include "Game/ShibLobbyGameMode.h"
#include "ShibLobbyPlayerState.generated.h"

enum class EShibClass : uint8;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUpdateGameRoomDelegate, float, LobbyTime, bool, bIsDedicatedServer, ELobbyStatus, LobbyStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FUpdateGameRoomCardDelegate, FString, PlayerName, EShibClass, SelectedShibClass, bool, bReady);

/**
 * 
 */
UCLASS()
class SHIBRUN_API AShibLobbyPlayerState : public AShibBasePlayerState
{
	GENERATED_BODY()
	
public:

	virtual void PostInitializeComponents() override;
	virtual void ClientInitialize(AController* C) override;
	
	// Setup custom replicated variables
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerSetIsReady(bool bReady);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ServerRequestGameRoomUpdate();

	UFUNCTION(Client, Reliable)
	void ClientUpdateGameRoom(float LobbyTime, bool bIsDedicatedServer, ELobbyStatus LobbyStatus);
	
	virtual void SetSelectedShibClass(EShibClass NewShibClass) override;
	
	// Player selected a class and is ready in the lobby
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing=OnRep_IsReady)
	bool bIsReady = false;

	// Update the game room players array
	UPROPERTY(BlueprintAssignable)
	FUpdateGameRoomDelegate UpdateGameRoomDelegate;

	// Update the player's card details in the game room
	UPROPERTY(BlueprintAssignable)
	FUpdateGameRoomCardDelegate UpdateGameRoomCardDelegate;
	
	UFUNCTION()
	void OnRep_IsReady();

	virtual void OnRep_PlayerName() override;
	
};
