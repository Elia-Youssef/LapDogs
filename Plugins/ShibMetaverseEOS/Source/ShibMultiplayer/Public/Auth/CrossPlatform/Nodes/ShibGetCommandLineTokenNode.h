#pragma once

#if EOS_HAS_AUTHENTICATION

#include "CoreMinimal.h"

#include "HttpModule.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"

namespace Redpoint::EOS::Auth::CrossPlatform
{

class SHIBMULTIPLAYER_API FShibGetCommandLineTokenNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FShibGetCommandLineTokenNode);
    FShibGetCommandLineTokenNode() = default;
    virtual ~FShibGetCommandLineTokenNode() = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FShibGetCommandLineTokenNode");
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
