// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ShibSave.generated.h"

USTRUCT(BlueprintType)
struct FShibSettings
{
	GENERATED_BODY()

	// General
	UPROPERTY(BlueprintReadWrite, Category = "General")
	float MouseSensitivity = 0.5f;
	
	UPROPERTY(BlueprintReadWrite, Category = "General")
	bool bInvertYAxis = true;
	
	UPROPERTY(BlueprintReadWrite, Category = "General")
	bool bShowAvatarNames = true;

	// Audio
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float MasterVolume = 1.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float MusicVolume = 1.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Audio")
	float EffectsVolume = 1.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	TEnumAsByte<EWindowMode::Type> WindowMode = EWindowMode::WindowedFullscreen;

	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	FIntPoint Resolution = FIntPoint(1920, 1080); // User game settings class default values are used for the resolution
	
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	int32 GraphicsQuality = 2; // Epic

	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	bool BenckmarkOptimizedGraphics = false; // When the user ran a benckmark to optimize its graphics, we don't want to use the default scalability value
	
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	float Brightness = 1.f;
	
	UPROPERTY(BlueprintReadWrite, Category = "Graphics")
	bool bMotionBlur = true;
	
	// Controls - CURRENTLY USING UE's BUILT IN SAVE
	UPROPERTY(BlueprintReadWrite, Category = "Controls")
	TMap<FName, FKey> KeyMappings = TMap<FName, FKey>{
		{FName("Pause"), EKeys::Escape},
		{FName("Forward"), EKeys::W},
		{FName("Left"), EKeys::A},
		{FName("Backward"), EKeys::S},
		{FName("Right"), EKeys::D},
		{FName("Jump"), EKeys::SpaceBar},
		{FName("Interact"), EKeys::F},
		{FName("Reload"), EKeys::R},
		{FName("LeftCorner"), EKeys::Q},
		{FName("RightCorner"), EKeys::E},
		{FName("Run"), EKeys::LeftShift}
	};
};

/**
 * 
 */
UCLASS()
class SHIBUINAVIGATION_API  UShibSave : public USaveGame
{
	GENERATED_BODY()

public:
	/**
	 * @return Default shib settings struct
	 */
	UFUNCTION(BlueprintPure, Category = "Settings")
	FShibSettings GetDefaultShibSettings() { return FShibSettings(); }
	
	UPROPERTY(BlueprintReadWrite, Category="Settings")
	FShibSettings ShibSettings;

	/**This will be saved to true after completing or skipping the initial tutorial level.*/
	UPROPERTY(BlueprintReadWrite, Category = "Tutorial")
	bool bSkipTutorial = false;

	/**This will be saved to true after completing or skipping the initial tournaments tutorial.*/
	UPROPERTY(BlueprintReadWrite, Category = "Tutorial")
	bool bSkipTournamentsTutorial = false;
};
