// Copyright Shiba Inu Games LLC.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "ShibAPIsSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Shib APIs"))
class SHIBAPIS_API UShibAPIsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UShibAPIsSettings();

	//////////// SERVER ////////////
	UPROPERTY(config, EditAnywhere, meta=(DisplayName="Dedicated Server API Key"), Category=Authentication)
	FString DedicatedServerApiKey = FString();

	//////////// GENERIC ////////////

	UPROPERTY(config, EditAnywhere, meta=(DisplayName="Generic API URL"), Category=Generic)
	FString ServerURL = FString();

	//////////// TOURNAMENTS ////////////

	UPROPERTY(config, EditAnywhere, meta=(DisplayName="Tournaments API Key"), Category=Authentication)
	FString TournamentsServerApiKey = FString();

	UPROPERTY(Config, EditAnywhere, meta=(DisplayName="Tournaments API URL"), Category=Tournaments)
	FString TournamentsServerURL = FString();

	UFUNCTION(BlueprintPure, Category="ShibAPIs", meta=(DisplayName="Get Shib APIs Settings"))
	static const UShibAPIsSettings* Get()
	{
		return GetDefault<UShibAPIsSettings>();
	}
};
