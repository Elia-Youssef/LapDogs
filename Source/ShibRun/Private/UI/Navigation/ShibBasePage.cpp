// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Navigation/ShibBasePage.h"
#include "Game/ShibBaseGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShibBaseController.h"

void UShibBasePage::NativeConstruct()
{
	GetShibBaseController();
	GetShibBasePS();
	GetShibGI();
	
	Super::NativeConstruct();

	BaseNavigation = ShibNavigation;
}

UShibGameInstance* UShibBasePage::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}

AShibBaseController* UShibBasePage::GetShibBaseController()
{
	if (!ShibBaseCtrl) ShibBaseCtrl = Cast<AShibBaseController>(GetOwningPlayer());
	return ShibBaseCtrl;
}

AShibBasePlayerState* UShibBasePage::GetShibBasePS()
{
	if (!ShibBasePS) ShibBasePS = GetOwningPlayerState<AShibBasePlayerState>();
	return ShibBasePS;
}

AShibBaseGameState* UShibBasePage::GetShibBaseGS()
{
	if (!ShibBaseGS) ShibBaseGS = Cast<AShibBaseGameState>(UGameplayStatics::GetGameState(GetOwningPlayer()));
	return ShibBaseGS;
}
