// Copyright Shiba Inu Games LLC.

#include "Game/ShibGameInstance.h"
#include "ShibSessionsEOS.h"
#include "ShibMultiplayer/Public/ShibUserEOS.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

void UShibGameInstance::Init()
{
	Super::Init();

	SessionsSubsystem = GetSubsystem<UShibSessionsEOS>();
	UserSubsystem = GetSubsystem<UShibUserEOS>();
}

void UShibGameInstance::Shutdown()
{
	// Handle session destruction if we are a dedicated server
	if (IsDedicatedServerInstance() && SessionsSubsystem)
	{
		SessionsSubsystem->CleanUpSessions();
	}
	
	Super::Shutdown();
}

void UShibGameInstance::ShibTravelToSession(APlayerController* Controller, FString Address)
{
	if (!Controller) return;
	Controller->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UShibGameInstance::PlayBackgroundMusic(USoundBase* NewSound, float FadeInTime, float VolumeMultiplier,
	float PitchMultiplier, float StartTime, USoundConcurrency* ConcurrencySettings, bool bPersistAcrossLevelTransition,
	bool bAutoDestroy)
{
	if (BackgroundMusic)
	{
		BackgroundMusic->SetVolumeMultiplier(VolumeMultiplier);
		BackgroundMusic->SetPitchMultiplier(PitchMultiplier);
		BackgroundMusic->bAutoDestroy = bAutoDestroy;
		BackgroundMusic->bIgnoreForFlushing = bPersistAcrossLevelTransition;
			
		if (ConcurrencySettings)
		{
			BackgroundMusic->ConcurrencySet.Empty();
			BackgroundMusic->ConcurrencySet.Add(ConcurrencySettings);
		}
		
		BackgroundMusic->SetSound(NewSound);
		
		if (FadeInTime > 0)
		{
			BackgroundMusic->FadeIn(FadeInTime, 1.f, StartTime);
		}
		else if (!BackgroundMusic->IsPlaying())
		{
			BackgroundMusic->Play();
		}
	}
	else
	{
		BackgroundMusic = UGameplayStatics::SpawnSound2D(GetWorld(), NewSound, VolumeMultiplier, PitchMultiplier, StartTime, ConcurrencySettings,bPersistAcrossLevelTransition, bAutoDestroy);
		BackgroundMusic->bCanPlayMultipleInstances = false;
		if (FadeInTime > 0)
		{
			BackgroundMusic->FadeIn(FadeInTime);
		}
	}
}

void UShibGameInstance::StopBackgroundMusic(float FadeOutTime)
{
	if (BackgroundMusic)
	{
		if (FadeOutTime > 0)
		{
			BackgroundMusic->FadeOut(FadeOutTime, 0);
		}
		else
		{
			BackgroundMusic->Stop();
		}
	}
}

void UShibGameInstance::SaveAbilityUpgrades(TArray<TObjectPtr<UShibAbility>> AbilityUpgradesToSave)
{
	if (!AbilityUpgradesToSave.IsEmpty())
	{
		for (auto AbilityToSave : AbilityUpgradesToSave)
		{
			SavedAbilityUpgrades.Add(AbilityToSave.GetClass());
		}
	}
}

bool UShibGameInstance::LoadAbilityUpgrades(TArray<TSubclassOf<UShibAbility>> &OutSavedAbilityUpgrades)
{
	if (!SavedAbilityUpgrades.IsEmpty())
	{
		OutSavedAbilityUpgrades = SavedAbilityUpgrades;
		return true;
	}
	else return false;
}