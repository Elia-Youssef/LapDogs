// Copyright Shiba Inu Games LLC.

#include "Player/ShibBaseController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Game/ShibBaseGameMode.h"
#include "Game/ShibGameInstance.h"
#include "Player/ShibPlayerState.h"
#include "UI/Navigation/ShibBaseNavigation.h"

void AShibBaseController::BeginPlay()
{
	Super::BeginPlay();

	CallCasts();
}

void AShibBaseController::InitializeShibController()
{
	Client_CreateHud();
}

void AShibBaseController::Client_CreateHud_Implementation()
{
	// Make sure we don't already have a UI
	// This can happen after a seamless travel
	if (PlayerUI)
	{
		PlayerUI->RemoveFromParent();
		PlayerUI=nullptr;
	}
	
	PlayerUI = Cast<UShibBaseNavigation>(CreateWidget<UUserWidget>(this, PlayerUI_Class));
	if (PlayerUI)
	{
		PlayerUI->AddToViewport();
		InitializeHud();
	}
}

void AShibBaseController::ClientWasKicked(const FText& KickReason)
{
	if (HasAuthority()) // We make sure that we don't kick the hosting player, only clients
	{
		if (!IsLocalPlayerController()) Client_WasKickedFromSession(KickReason);
	}
	else
	{
		InternalClientWasKickedFromSession(KickReason);
	}
}

void AShibBaseController::Client_WasKickedFromSession_Implementation(const FText& KickReason)
{
	InternalClientWasKickedFromSession(KickReason);
}

void AShibBaseController::OnNetCleanup(UNetConnection* Connection)
{
	// Handle player logout on the server side
	if (GetLocalRole() == ROLE_Authority && PlayerState != NULL)
	{
		AShibBaseGameMode* GameMode = Cast<AShibBaseGameMode>(GetWorld()->GetAuthGameMode());
		if (IsValid(GameMode))
		{
			GameMode->PreLogout(this);
		}
	}

	Super::OnNetCleanup(Connection);
}

void AShibBaseController::CallCasts()
{
	GetShibGI();
	GetShibBasePS();
	GetShibBaseGM();
}

TObjectPtr<UShibGameInstance> AShibBaseController::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}

TObjectPtr<AShibBasePlayerState> AShibBaseController::GetShibBasePS()
{
	if (!ShibBasePS) ShibBasePS = Cast<AShibBasePlayerState>(PlayerState);
	return ShibBasePS;
}

TObjectPtr<AShibBaseGameMode> AShibBaseController::GetShibBaseGM()
{
	if (!ShibBaseGM) ShibBaseGM = Cast<AShibBaseGameMode>(UGameplayStatics::GetGameMode(this));
	return ShibBaseGM;
}