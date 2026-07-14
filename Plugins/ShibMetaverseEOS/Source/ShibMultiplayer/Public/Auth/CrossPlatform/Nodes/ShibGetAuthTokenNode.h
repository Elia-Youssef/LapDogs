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
class SHIBMULTIPLAYER_API FShibGetAuthTokenNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FShibGetAuthTokenNode);
    FShibGetAuthTokenNode() = default;
    virtual ~FShibGetAuthTokenNode() = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FShibGetAuthTokenNode");
    }
    
private:
    void SendHttpRequest(FString URL, TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone);

    void OnHttpResponse(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        bool bConnectedSuccessfully,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone);

    FTimerHandle RequestHandle;
};

}

#endif
