// Copyright Shiba Inu Games LLC.

#include "Player/ShibLobbyPlayerState.h"
#include "ShibAPIsSubsystem.h"
#include "Character/ShibCharacter.h"
#include "Game/ShibGameInstance.h"
#include "Game/ShibLobbyGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Utils/ShibTypes.h"

void AShibLobbyPlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority())
	{
		// This next part is for saving the shib user id of the hosting player in listen server mode ONLY
		// Check if the player controller is locally controlled by the server first
		if (!GetPlayerController()) return;
		if (GetPlayerController()->IsLocalPlayerController())
		{
			if (auto GI = GetShibGI(); GI->bUseShibApis) // Check if Shib APIs is enable
			{
				// Save the shib user id for the hosting player
				if (auto ShibAPIs = GetGameInstance()->GetSubsystem<UShibAPIsSubsystem>())
				{
					CachedShibUserId = ShibAPIs->UserInfo.User.Id;
				}
			}
		}
	}
}

void AShibLobbyPlayerState::ClientInitialize(AController* C)
{
	//This function is only executed on the client when the player state gets replicated on the local player controller
	Super::ClientInitialize(C);

	if (auto GI = GetShibGI(); ShibGI->bUseShibApis) // Check if Shib APIs is enable
	{
		// Send the shib user id of the client to the server
		// This variable will then be replicated to the owner client only
		if (auto ShibAPIs = GetGameInstance()->GetSubsystem<UShibAPIsSubsystem>())
		{
			Server_ShareShibUserId(ShibAPIs->UserInfo.User.Id);
		}
	}
}

void AShibLobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShibLobbyPlayerState, bIsReady);
}

void AShibLobbyPlayerState::ServerRequestGameRoomUpdate_Implementation()
{
	if (auto* ShibLobbyGM = Cast<AShibLobbyGameMode>(GetShibBaseGM()))
		ShibLobbyGM->ForceUpdateGameRoom(this);
}

void AShibLobbyPlayerState::SetSelectedShibClass(EShibClass NewShibClass)
{
	Super::SetSelectedShibClass(NewShibClass);
	
	UpdateGameRoomCardDelegate.Broadcast(GetPlayerName(), SelectedShibClass, bIsReady);
}

void AShibLobbyPlayerState::ServerSetIsReady_Implementation(const bool bReady)
{
	auto* ShibLobbyGM = Cast<AShibLobbyGameMode>(GetShibBaseGM());
	// Notify the Game Mode that we are ready
	// The game mode will take care of setting the value of our bIsReady variable
	ShibLobbyGM->NotifyPlayerIsReadyToStart(this, bReady);
	
	UpdateGameRoomCardDelegate.Broadcast(GetPlayerName(), SelectedShibClass, bIsReady);
}

void AShibLobbyPlayerState::ClientUpdateGameRoom_Implementation(float LobbyTime, bool bIsDedicatedServer, ELobbyStatus LobbyStatus)
{
	UpdateGameRoomDelegate.Broadcast(LobbyTime, bIsDedicatedServer, LobbyStatus);
}

void AShibLobbyPlayerState::OnRep_IsReady()
{
	UpdateGameRoomCardDelegate.Broadcast(GetPlayerName(), SelectedShibClass, bIsReady);
}

void AShibLobbyPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	
	UpdateGameRoomCardDelegate.Broadcast(GetPlayerName(), SelectedShibClass, bIsReady);
	if (GetShibCharacter()) ShibCharacter->UpdatePlayerNameWidget();
}
