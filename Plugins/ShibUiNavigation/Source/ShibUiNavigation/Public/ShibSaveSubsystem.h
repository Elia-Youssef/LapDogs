// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShibSave.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ShibSaveSubsystem.generated.h"

#define SHIB_MV_SAVE_SLOT FString("shib_mv_save")

struct FShibSettings;
class UShibSave;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSaveDelegate);

/**
 * 
 */
UCLASS()
class SHIBUINAVIGATION_API  UShibSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

	/**
	* Called in the game instance Init
	*/
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	/**
	 * Called in the game instance Init (used to be, now it's called on Initialize in ShibSaveSubsystem)
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void Load();

	/**
	 * Saves the ShibSave object
	 */
	UFUNCTION(BlueprintCallable, Category = "Save")
	void Save();

	/**
	 * Returns saved shib settings
	 * @return Saved Shib Settings
	 */
	UFUNCTION(BlueprintPure, Category = "Save|Settings")
	FShibSettings& GetShibSettings() { return ShibSave->ShibSettings; }
	
	UPROPERTY(BlueprintReadWrite, Category = "Save")
	TObjectPtr<UShibSave> ShibSave;

	UPROPERTY(BlueprintAssignable)
	FOnSaveDelegate OnSaveDelegate;
};
