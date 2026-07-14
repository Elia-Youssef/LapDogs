// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Navigation/ShibBaseNavigation.h"

#include "Game/ShibGameInstance.h"
#include "Game/ShibGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShibBaseController.h"

void UShibBaseNavigation::NativeConstruct()
{
	GetShibBaseController();
	GetShibPS();
	
	Super::NativeConstruct();
}

UShibGameInstance* UShibBaseNavigation::GetShibGI()
{
	if (!ShibGI) ShibGI = GetGameInstance<UShibGameInstance>();
	return ShibGI;
}

AShibBaseController* UShibBaseNavigation::GetShibBaseController()
{
	if (!ShibBaseCtrl) ShibBaseCtrl = Cast<AShibBaseController>(GetOwningPlayer());
	return ShibBaseCtrl;
}

AShibPlayerState* UShibBaseNavigation::GetShibPS()
{
	if (!ShibPS) ShibPS = GetOwningPlayerState<AShibPlayerState>();
	return ShibPS;
}

AShibGameState* UShibBaseNavigation::GetShibGS()
{
	if (!ShibGS) ShibGS = Cast<AShibGameState>(UGameplayStatics::GetGameState(GetOwningPlayer()));
	return ShibGS;
}
