// Copyright Shiba Inu Games LLC.
#pragma once

#include "CoreMinimal.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Auth::Graphs
{
#define EOS_AUTH_GRAPH_SHIB FName(TEXT("Shib"))

class FAuthenticationGraphShib : public FAuthenticationGraph
{
protected:
    virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(const TSharedRef<FAuthenticationGraphState> &InitialState) const override;

public:
    UE_NONCOPYABLE(FAuthenticationGraphShib);
    FAuthenticationGraphShib() = default;
    virtual ~FAuthenticationGraphShib() = default;
    
    static void Register();
};
}

#endif
