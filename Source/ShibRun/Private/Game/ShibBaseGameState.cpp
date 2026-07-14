// Copyright Shiba Inu Games LLC.

#include "Game/ShibBaseGameState.h"
#include "Game/ShibGameMode.h"
#include "Kismet/GameplayStatics.h"

// ---- Getters ----
TObjectPtr<AShibGameMode> AShibBaseGameState::GetShibGM()
{
	if (!ShibGM) ShibGM = Cast<AShibGameMode>(UGameplayStatics::GetGameMode(this));
	return ShibGM;
}