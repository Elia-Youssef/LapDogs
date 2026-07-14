// Copyright Shiba Inu Games LLC.

#pragma once

#include "Auth/PlatformIntegration/ShibCredentialInfo.h"
#include "RedpointEOSAuth/AuthenticationGraphNode.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Platform::Integration
{
using namespace ::Redpoint::EOS::Auth;
	
class SHIBMULTIPLAYER_API FShibGetExternalCredentialsNode : public FAuthenticationGraphNode
{
private:
	void OnCredentialsObtained(
		bool bWasSuccessful,
		FShibCredentialInfo ObtainedCredentials,
		TSharedRef<FAuthenticationGraphState> InState,
		FAuthenticationGraphNodeOnDone InOnDone);

public:
	FShibGetExternalCredentialsNode() = default;
	UE_NONCOPYABLE(FShibGetExternalCredentialsNode);

	virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

	virtual FString GetDebugName() const override
	{
		return TEXT("FShibGetExternalCredentialsNode");
	}
};
}
#endif
