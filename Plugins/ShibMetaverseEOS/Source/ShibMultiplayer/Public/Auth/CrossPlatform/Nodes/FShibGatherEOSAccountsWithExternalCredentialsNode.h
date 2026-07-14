// Copyright Shiba Inu Games LLC.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Auth::CrossPlatform
{
class SHIBMULTIPLAYER_API FShibGatherEOSAccountsWithExternalCredentialsNode : public FAuthenticationGraphNode
{
public:
	UE_NONCOPYABLE(FShibGatherEOSAccountsWithExternalCredentialsNode);
	FShibGatherEOSAccountsWithExternalCredentialsNode() = default;
	virtual ~FShibGatherEOSAccountsWithExternalCredentialsNode() = default;

	virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

	virtual FString GetDebugName() const override
	{
		return TEXT("FShibGatherEOSAccountsWithExternalCredentialsNode");
	}

};
}

#endif