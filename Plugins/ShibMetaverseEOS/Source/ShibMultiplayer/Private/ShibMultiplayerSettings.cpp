// Copyright Shiba Inu Games LLC.


#include "ShibMultiplayerSettings.h"

UShibMultiplayerSettings::UShibMultiplayerSettings(const FObjectInitializer& Obj)
{
	TokenArgKey = FString();
	UserIdArgKey = FString();
	
	// bUseWebview = false;
	// ServerAuthenticationURL = FString();
	// ServerRequestsRate = 2.f;
	// ShibAuthURL = FString();
}

// bool UShibMultiplayerSettings::GetUseWebviewAuthentication()
// {
// 	const auto* Settings = GetMutableDefault<UShibMultiplayerSettings>();
// 	if (!Settings) return false;
//
// 	return Settings->bUseWebview;
// }
//
// FString UShibMultiplayerSettings::GetShibAuthURL()
// {
// 	const auto* Settings = GetMutableDefault<UShibMultiplayerSettings>();
// 	if (!Settings) return FString();
//
// 	return Settings->ShibAuthURL;
// }
