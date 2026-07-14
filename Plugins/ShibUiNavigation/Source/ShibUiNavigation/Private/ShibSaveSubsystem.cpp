// Fill out your copyright notice in the Description page of Project Settings.

#include "ShibSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UShibSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Load();
}

void UShibSaveSubsystem::Load()
{
	if (!ShibSave)
		ShibSave = Cast<UShibSave>(UGameplayStatics::LoadGameFromSlot(SHIB_MV_SAVE_SLOT, 0));
	if (!ShibSave)
		ShibSave = Cast<UShibSave>(UGameplayStatics::CreateSaveGameObject(UShibSave::StaticClass()));
}

void UShibSaveSubsystem::Save()
{
	UGameplayStatics::SaveGameToSlot(ShibSave, SHIB_MV_SAVE_SLOT, 0);
	OnSaveDelegate.Broadcast();
}
