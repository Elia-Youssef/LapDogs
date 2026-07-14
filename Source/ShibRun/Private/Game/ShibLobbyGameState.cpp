// Copyright Shiba Inu Games LLC.

#include "Game/ShibLobbyGameState.h"

void AShibLobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
	PlayerArrayModified.Broadcast();
}

void AShibLobbyGameState::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
	PlayerArrayModified.Broadcast();
}
