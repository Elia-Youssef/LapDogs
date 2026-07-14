// Copyright Shiba Inu Games LLC.

#include "Auth/CrossPlatform/Nodes/ShibGetExternalCredentialsNode.h"
#include "Auth/PlatformIntegration/ShibCredentialObtainer.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "Auth/PlatformIntegration/ShibCredentialInfo.h"
#include "Auth/PlatformIntegration/ShibExternalCredentials.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Platform::Integration
{
	
void FShibGetExternalCredentialsNode::OnCredentialsObtained(bool bWasSuccessful,
	FShibCredentialInfo ObtainedCredentials,
	TSharedRef<FAuthenticationGraphState> InState,
	FAuthenticationGraphNodeOnDone InOnDone)
{
	if (!bWasSuccessful)
	{
		InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
		return;
	}

	InState->AvailableExternalCredentials.Add(MakeShared<FShibExternalCredentials>(ObtainedCredentials));
	InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

void FShibGetExternalCredentialsNode::Execute(TSharedRef<FAuthenticationGraphState> State,
	FAuthenticationGraphNodeOnDone OnDone)
{
	FShibCredentialObtainer::StartFromAuthenticationGraph(
	   State,
	   FShibCredentialObtainer::FOnCredentialObtained::CreateSP(
		   this,
		   &FShibGetExternalCredentialsNode::OnCredentialsObtained,
		   State,
		   OnDone));
}
	
}
#endif