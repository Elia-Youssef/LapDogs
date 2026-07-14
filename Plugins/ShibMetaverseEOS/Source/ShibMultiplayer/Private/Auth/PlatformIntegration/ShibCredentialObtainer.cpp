// Copyright Shiba Inu Games LLC.

#include "Auth/PlatformIntegration/ShibCredentialObtainer.h"
#include "OnlineSubsystemUtils.h"
#include "ShibMultiplayerSettings.h"
#include "ShibUserEOS.h"
#include "Auth/AuthenticationHelpersShib.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Platform::Integration
{
	
FShibCredentialObtainer::FShibCredentialObtainer(const FShibCredentialObtainer::FOnCredentialObtained& Cb)
	: FAuthenticationCredentialObtainer(Cb)
	, SessionTicket(TEXT(""))
	, bSuccess(false)
	, bComplete(false)
{
}

bool FShibCredentialObtainer::Init(UWorld* World, int32 LocalUserNum)
{
	if (World == nullptr)
	{
		this->EmitError(TEXT("Could not authenticate with Steam, because the UWorld* pointer was null."));
		this->Done(false, FShibCredentialInfo());
		return false;
	}

	IOnlineSubsystem *OSSSubsystem = Online::GetSubsystem(World);;
	if (OSSSubsystem == nullptr)
	{
		this->EmitError(TEXT("Could not authenticate with online subsystem, because the subsystem was not "
							 "available. Check that OSS is enabled in your DefaultEngine.ini file."));
		this->Done(false, FShibCredentialInfo());
		return false;
	}

	TSharedPtr<IOnlineIdentity, ESPMode::ThreadSafe> OSSIdentity =
		StaticCastSharedPtr<IOnlineIdentity>(OSSSubsystem->GetIdentityInterface());
	if (!OSSIdentity.IsValid())
	{
		this->EmitError(TEXT("Could not authenticate with Shib, because the identity interface was not available."));
		this->Done(false, FShibCredentialInfo());
		return false;
	}
	
	if (!IsValid(GetMutableDefault<UShibMultiplayerSettings>()))
	{
		this->EmitError(TEXT("Could not authenticate with Shib, because multiplayer settings was not available. Check your project settings."));
		this->Done(false, FShibCredentialInfo());
		return false;
	}

	this->OSSIdentityWk = OSSIdentity;

	GetCmdToken();
	
	return true;
}

void FShibCredentialObtainer::GetCmdToken()
{
	auto Settings = GetMutableDefault<UShibMultiplayerSettings>();
	auto OSSIdentity = this->OSSIdentityWk.Pin();

	if (!Settings || !OSSIdentity.IsValid())
	{
		this->EmitError(TEXT("Could not authenticate with Shib, because Shib OSS or Shib settings are no longer available."));
		this->bSuccess = false;
		this->bComplete = true;
		return;
	}
	
	if (Settings->TokenArgKey.IsEmpty() || Settings->UserIdArgKey.IsEmpty())
	{
		this->EmitError(TEXT("Could not authenticate with Shib, because token or user data argument key is invalid. Check your project settings."));
		this->bSuccess = false;
		this->bComplete = true;
		return;
	}

	if (FString Arg; FParse::Value(FCommandLine::Get(), *FString::Printf(TEXT("%s="), *Settings->TokenArgKey), Arg))
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s: %s"), *Settings->TokenArgKey, *Arg);
		this->SessionTicket = Arg;
	}
	else
	{
		this->EmitError((FString::Printf(TEXT("Could not authenticate with Shib, because token argument for key `%s`. is invalid."), *Arg)));
		this->bSuccess = false;
		this->bComplete = true;
		return;
	}

	// Not sure at this point if we should use the user id from the Online subsystem instead
	//this->UserId = OSSIdentity->GetUniquePlayerId(0)->ToString();
	
	if (FString Arg; FParse::Value(FCommandLine::Get(), *FString::Printf(TEXT("%s="), *Settings->UserIdArgKey), Arg))
	{
		UE_LOG(LogTemp, Verbose, TEXT("%s: %s"), *Settings->UserIdArgKey, *Arg);
		this->UserId = Arg;
	}
	
	this->bSuccess = true;
	this->bComplete = true;
}
	
bool FShibCredentialObtainer::Tick(float DeltaSeconds)
{
	if (!this->bComplete) return true;

	if (this->bSuccess)
	{
		TMap<FString, FString> ShibAuthAttributes;
		ShibAuthAttributes.Add(TEXT("authenticatedWith"), TEXT("Shib"));
		ShibAuthAttributes.Add(TEXT("shib.UserId"), this->UserId);
		ShibAuthAttributes.Add(TEXT("shib.Token"), this->SessionTicket);
		
		FShibCredentialInfo Info;
		Info.UserId = this->UserId;
		Info.SessionTicket = this->SessionTicket;
		Info.AuthAttributes = ShibAuthAttributes;
		this->Done(true, Info);
	}
	else
	{
		this->Done(false, FShibCredentialInfo());
	}
	return false;
}
}

#endif