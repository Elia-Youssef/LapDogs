// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibSessionsEOS.h"
#include "Engine/GameInstance.h"
#include "ShibGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMapLoadingStatus);

class UShibUserAPIs;
class UShibUserEOS;
enum class EShibClass : uint8;
struct FBlueprintSessionResult;
class UShibUserSubsystem;
class UShibAbility;

UCLASS()
class SHIBRUN_API UShibGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	virtual void Shutdown() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Shib Game Instance|Game Session")
	void InitGameSession();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Shib Game Instance|Game Session")
	void TerminateGameSession();

	UFUNCTION(BlueprintCallable)
	void ShibTravelToSession(APlayerController* Controller, FString Address);

	UFUNCTION(BlueprintCallable)
	virtual void PlayBackgroundMusic(USoundBase* NewSound, float FadeInTime = 0.f, float VolumeMultiplier = 1.f, float PitchMultiplier = 1.f, float StartTime = 0.f, USoundConcurrency* ConcurrencySettings = nullptr, bool bPersistAcrossLevelTransition = false, bool bAutoDestroy = true);

	UFUNCTION(BlueprintCallable)
	virtual void StopBackgroundMusic(float FadeOutTime);
	
	/**
	* Save the ability upgrades granted during a game
	* We generally save the upgrades before a map change and reload them in the ability system after 
	*/
	void SaveAbilityUpgrades(TArray<TObjectPtr<UShibAbility>> AbilityUpgradesToSave);

	/**
	* Return the ability ugrades saved previously
	* Return noting if no upgrade is saved
	*/
	UFUNCTION(BlueprintCallable, Category = "Shib Game Instance|Ability")
	bool LoadAbilityUpgrades(TArray<TSubclassOf<UShibAbility>> &OutSavedAbilityUpgrades);
	
	/**
	* DEVELOPMENT ONLY, Enable or disable EOS Redpoint in the game
	* This must be set to true when packaging the game
	*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildSettings")
	bool bUseEosSubsystem=true;

	/**
	* DEVELOPMENT ONLY, Enable or disable Shib APIs in the game
	* This must be set to true when packaging the game
	*/ 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildSettings")
	bool bUseShibApis=true;

	UPROPERTY()
	TObjectPtr<UShibSessionsEOS> SessionsSubsystem;

	UPROPERTY()
	TObjectPtr<UShibUserEOS> UserSubsystem;
	
	// Selected character class
	UPROPERTY(BlueprintReadWrite)
	EShibClass SelectedShibClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildSettings")
	int32 DefaultNumConnections = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 NumConnectedControllers = 0;
private:
	
	UPROPERTY()
	UAudioComponent* BackgroundMusic;
	
	TArray<TSubclassOf<UShibAbility>> SavedAbilityUpgrades;
};
