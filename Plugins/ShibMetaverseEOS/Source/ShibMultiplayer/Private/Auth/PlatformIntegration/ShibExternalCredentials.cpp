// Copyright Shiba Inu Games LLC.

#include "Auth/PlatformIntegration/ShibExternalCredentials.h"
#include "Auth/PlatformIntegration/ShibCredentialObtainer.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/SubsystemNames.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Platform::Integration
{
	
FShibExternalCredentials::FShibExternalCredentials(const FShibCredentialInfo& InCredentialInfo)
	: CredentialInfo(InCredentialInfo)
{
}

FText FShibExternalCredentials::GetProviderDisplayName() const
{
	return NSLOCTEXT("OnlineSubsystemEOSAuthShib", "Platform_Shib", "Shib");
}

FString FShibExternalCredentials::GetType() const
{
	return TEXT("OPENID_ACCESS_TOKEN");
}

FString FShibExternalCredentials::GetId() const
{
	return this->CredentialInfo.UserId;
}

FString FShibExternalCredentials::GetToken() const
{
	return this->CredentialInfo.SessionTicket;
}

TMap<FString, FString> FShibExternalCredentials::GetAuthAttributes() const
{
	return this->CredentialInfo.AuthAttributes;
}

FName FShibExternalCredentials::GetNativeSubsystemName() const
{
	return NAME_None;
}

void FShibExternalCredentials::OnCredentialsRefreshed(
	bool bWasSuccessful,
	FShibCredentialInfo NewCredentials,
	FOnlineExternalCredentialsRefreshComplete OnComplete)
{
	if (!bWasSuccessful)
	{
		OnComplete.ExecuteIfBound(false);
		return;
	}

	this->CredentialInfo = MoveTemp(NewCredentials);
	OnComplete.ExecuteIfBound(true);
}
	
void FShibExternalCredentials::Refresh(
	TSoftObjectPtr<UWorld> InWorld,
	int32 LocalUserNum,
	FOnlineExternalCredentialsRefreshComplete OnComplete)
{
	FShibCredentialObtainer::StartFromCredentialRefresh(
	InWorld,
	LocalUserNum,
	FShibCredentialObtainer::FOnCredentialObtained::CreateSP(
		this,
		&FShibExternalCredentials::OnCredentialsRefreshed,
		OnComplete));
}


	
}
#endif
