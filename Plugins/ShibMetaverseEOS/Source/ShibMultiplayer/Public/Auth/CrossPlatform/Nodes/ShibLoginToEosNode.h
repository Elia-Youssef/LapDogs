// Copyright June Rhodes 2024. All Rights Reserved.

#pragma once
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/Auth/CopyUserAuthToken.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"

#if EOS_HAS_AUTHENTICATION

#include "CoreMinimal.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"

class FOnlineSubsystemEOS;

namespace Redpoint::EOS::Auth::CrossPlatform
{

class SHIBMULTIPLAYER_API FShibLoginToEosNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FShibLoginToEosNode);
    FShibLoginToEosNode() = default;
    virtual ~FShibLoginToEosNode() = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FShibLoginToEosNode");
    }
};

}

#endif
