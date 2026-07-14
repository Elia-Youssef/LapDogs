#pragma once

#if EOS_HAS_AUTHENTICATION

#include "CoreMinimal.h"

#include "HttpModule.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"

namespace Redpoint::EOS::Auth::CrossPlatform
{

/**
 * DEPRECATED
 * Kept as reference
 */
class SHIBMULTIPLAYER_API FShibGetWebviewTokenNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FShibGetWebviewTokenNode);
    FShibGetWebviewTokenNode() = default;
    virtual ~FShibGetWebviewTokenNode() = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FShibGetWebviewTokenNode");
    }
    
};

}

#endif
