// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShibMultiplayer.h"
#include "Modules/ModuleManager.h"

#if WITH_EDITOR
	#include "ISettingsModule.h"
#endif

#if EOS_HAS_AUTHENTICATION
	#include "Auth/CrossPlatform/ShibCrossPlatformAccountProvider.h"
#endif

#include "OnlineSubsystemNames.h"
#include "ShibMultiplayerSettings.h"
#include "Auth/CrossPlatform/Nodes/ShibGetExternalCredentialsNode.h"
#include "Auth/Graphs/AuthenticationGraphShib.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/SubsystemNames.h"
#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/Graphs/AuthenticationGraphOnlineSubsystem.h"
#define LOCTEXT_NAMESPACE "FShibMetaverseEOSModule"

void FShibMultiplayerModule::StartupModule()
{
	FNetworkVersion::IsNetworkCompatibleOverride.BindLambda([](const uint32 LocalNetworkVersion, const uint32 RemoteNetworkVersion){ return true; });

#if WITH_EDITOR
	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Game", "ShibMultiplayer",
			LOCTEXT("RuntimeSettingsName", "Shib Multiplayer"), LOCTEXT("RuntimeSettingsDescription", "Configure shib multiplayer settings"),
			GetMutableDefault<UShibMultiplayerSettings>());
	}
#endif
	
#if EOS_HAS_AUTHENTICATION

	using Redpoint::EOS::Auth::CrossPlatform::FShibCrossPlatformAccountProvider;
	//using Redpoint::EOS::Auth::Graphs::FAuthenticationGraphShib;
	//using Redpoint::EOS::Auth::FAuthenticationGraphRegistry;
	using namespace ::Redpoint::EOS::Auth::Graphs;
	using namespace Redpoint::EOS::Platform::Integration;
/*
	FAuthenticationGraphOnlineSubsystem::RegisterForCustomPlatform(
			FName(TEXT("Shib")),
			NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_Shib", "Shib Only"),
			REDPOINT_EOS_SUBSYSTEM,
			EOS_EExternalCredentialType::EOS_ECT_OPENID_ACCESS_TOKEN,
			TEXT("Shib"),
			TEXT("shib.Token"),
			MakeShared<FShibGetExternalCredentialsNode>());
	*/
	FAuthenticationGraphRegistry::RegisterCrossPlatformAccountProvider(
        FName(TEXT("Shib")),
        MakeShared<FShibCrossPlatformAccountProvider>()
	);
	
	FAuthenticationGraphShib::Register();
	
#endif
}

void FShibMultiplayerModule::ShutdownModule()
{
	FNetworkVersion::IsNetworkCompatibleOverride.Unbind();

#if WITH_EDITOR
	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "ShibMultiplayer");
	}
#endif
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FShibMultiplayerModule, ShibMultiplayer)