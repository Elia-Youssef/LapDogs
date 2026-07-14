// Copyright Shiba Inu Games LLC.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

#include "ShibMultiplayerSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "ShibCredentialInfo.h"
#include "RedpointEOSAuth/AuthenticationCredentialObtainer.h"

namespace Redpoint::EOS::Platform::Integration
{
using namespace ::Redpoint::EOS::Auth;
	
class SHIBMULTIPLAYER_API FShibCredentialObtainer
	: public FAuthenticationCredentialObtainer<FShibCredentialObtainer, FShibCredentialInfo>
{
private:

	FString UserId;
	FString SessionTicket;
	bool bSuccess;
	bool bComplete;
	TWeakPtr<IOnlineIdentity, ESPMode::ThreadSafe> OSSIdentityWk;

private:
	void GetCmdToken();
	
public:
	FShibCredentialObtainer(const FShibCredentialObtainer::FOnCredentialObtained &Cb);
	UE_NONCOPYABLE(FShibCredentialObtainer);

	virtual bool Init(UWorld *World, int32 LocalUserNum) override;
	virtual bool Tick(float DeltaSeconds) override;
	
};
}
#endif