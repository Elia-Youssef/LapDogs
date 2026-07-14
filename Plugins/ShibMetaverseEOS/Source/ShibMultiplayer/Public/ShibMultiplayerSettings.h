// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShibMultiplayerSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game)
class SHIBMULTIPLAYER_API UShibMultiplayerSettings : public UObject
{
	GENERATED_BODY()

public:
	UShibMultiplayerSettings(const FObjectInitializer& Obj);

	/**
	 * `Token` argument key passed when the game is launched
	 */
	UPROPERTY(Config, EditAnywhere, Category="Authentication Graph|Command Line")
	FString TokenArgKey;
	
	/**
	 * `UserData` argument key passed when the game is launched.
	 * Should be a string of JSON format.
	 */
	UPROPERTY(Config, EditAnywhere, Category="Authentication Graph|Command Line")
	FString UserIdArgKey;
	
	// ========================== KEPT AS REFERENCE ==========================
	//
	// /**
	//  * Open authentication URL in-game or in browser
	//  */
	// UPROPERTY(EditAnywhere, Category="Authentication Graph")
	// bool bUseWebview;
	//
	// /**
	//  * Server URL to ask for authentication
	//  */
	// UPROPERTY(EditAnywhere, Category="Authentication Graph|Server")
	// FString ServerAuthenticationURL;
	//
	// /**
	//  * Server requests rate in seconds
	//  */
	// UPROPERTY(EditAnywhere, Category="Authentication Graph|Server")
	// float ServerRequestsRate;
	//
	// /**
	//  * ShibAuth SDK URL
	//  */
	// UPROPERTY(EditAnywhere, Category="Authentication Graph")
	// FString ShibAuthURL;
	//
	// UFUNCTION(BlueprintCallable)
	// static bool GetUseWebviewAuthentication();
	//
	// UFUNCTION(BlueprintCallable)
	// static FString GetShibAuthURL();
};
