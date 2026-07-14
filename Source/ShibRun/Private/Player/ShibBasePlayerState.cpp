// Copyright Shiba Inu Games LLC.

#include "Player/ShibBasePlayerState.h"
#include "Character/ShibCharacter.h"
#include "Game/ShibBaseGameMode.h"
#include "Game/ShibGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShibBaseController.h"

AShibBasePlayerState::AShibBasePlayerState()
{
	NetUpdateFrequency = 30.0f;
}

void AShibBasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AShibBasePlayerState, SelectedShibClass);
	
	FDoRepLifetimeParams Params;
	Params.Condition = COND_OwnerOnly;
	DOREPLIFETIME_WITH_PARAMS(AShibBasePlayerState, CachedShibUserId, Params);
}

void AShibBasePlayerState::Client_LoadSelectedShibClassFromInstance_Implementation()
{
	if (GetShibGI()) 
	{
		SelectedShibClass = ShibGI->SelectedShibClass;
		Server_SetSelectedShibClass(SelectedShibClass);
	}
}

void AShibBasePlayerState::SetSelectedShibClass(EShibClass NewShibClass)
{
	if (HasAuthority())
	{
		SelectedShibClass = NewShibClass;
		return;
	}
	
	SelectedShibClass = NewShibClass;
	Server_SetSelectedShibClass(NewShibClass);
}

void AShibBasePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	auto* NewPlayerState = Cast<AShibBasePlayerState>(PlayerState);
	if (NewPlayerState == nullptr) return;

	// Transfer all the info from the old player state to the new player state here
	// This is executed on the old player state before it is destroyed

	// Selected character class
	NewPlayerState->SelectedShibClass = SelectedShibClass;
	// Cached Shib user ID
	NewPlayerState->CachedShibUserId = CachedShibUserId;
	// Last Checkpoint
	NewPlayerState->CheckpointTag = CheckpointTag;
}

void AShibBasePlayerState::Server_ShareShibUserId_Implementation(int32 ShibUserId)
{
	CachedShibUserId = ShibUserId;
}

bool AShibBasePlayerState::Server_ShareShibUserId_Validate(int32 ShibUserId)
{
	if (ShibUserId <= 0)
	{
		return false;
	}
	
	return true;
}

void AShibBasePlayerState::Server_SetSelectedShibClass_Implementation(EShibClass NewShibClass)
{
	SetSelectedShibClass(NewShibClass);
}

TObjectPtr<UShibGameInstance> AShibBasePlayerState::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}

TObjectPtr<AShibBaseController> AShibBasePlayerState::GetShibCtrl()
{
	if (!ShibCtrl) ShibCtrl = Cast<AShibBaseController>(GetPlayerController());
	return ShibCtrl;
}

TObjectPtr<AShibCharacter> AShibBasePlayerState::GetShibCharacter()
{
	if (!ShibCharacter) ShibCharacter = Cast<AShibCharacter>(GetPawn());
	return ShibCharacter;
}

TObjectPtr<AShibBaseGameMode> AShibBasePlayerState::GetShibBaseGM()
{
	if (!ShibBaseGM) ShibBaseGM = Cast<AShibBaseGameMode>(UGameplayStatics::GetGameMode(this));
	return ShibBaseGM;
}
