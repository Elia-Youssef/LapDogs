// Copyright Shiba Inu Games LLC.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "Auth/PlatformIntegration/ShibCredentialInfo.h"

namespace Redpoint::EOS::Platform::Integration
{
using namespace ::Redpoint::EOS::Auth;
	
class SHIBMULTIPLAYER_API FShibExternalCredentials : public IOnlineExternalCredentials
{
private:
	FShibCredentialInfo CredentialInfo;

	void OnCredentialsRefreshed(
	bool bWasSuccessful,
	FShibCredentialInfo NewCredentials,
	FOnlineExternalCredentialsRefreshComplete OnComplete);

public:
	FShibExternalCredentials(const FShibCredentialInfo &InCredentialInfo);
	UE_NONCOPYABLE(FShibExternalCredentials);
	virtual ~FShibExternalCredentials() = default;
	virtual FText GetProviderDisplayName() const override;
	virtual FString GetType() const override;
	virtual FString GetId() const override;
	virtual FString GetToken() const override;
	virtual TMap<FString, FString> GetAuthAttributes() const override;
	virtual FName GetNativeSubsystemName() const override;
	virtual void Refresh(
		TSoftObjectPtr<UWorld> InWorld,
		int32 LocalUserNum,
		FOnlineExternalCredentialsRefreshComplete OnComplete) override;
};
	
}
#endif