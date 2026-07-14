// Copyright Shiba Inu Games LLC.

#include "Auth/Graphs/AuthenticationGraphShib.h"
#include "RedpointEOSAuth/Nodes/FailAuthenticationNode.h"
#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "Auth/CrossPlatform/ShibCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_LoginComplete.h"
#include "RedpointEOSAuth/Nodes/BailIfAlreadyAuthenticatedNode.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Auth::Graphs
{
using namespace Redpoint::EOS::Auth::Nodes;
using namespace Redpoint::EOS::Auth::CrossPlatform;

TSharedRef<FAuthenticationGraphNode> FAuthenticationGraphShib::CreateGraph(const TSharedRef<FAuthenticationGraphState> &InitialState) const
{
	if (InitialState->CrossPlatformAccountProvider.IsValid())
	{
		return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
			->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
			->Add(InitialState->CrossPlatformAccountProvider->GetNonInteractiveAuthenticationSequence());
	}
	else
	{
		return MakeShared<FFailAuthenticationNode>(TEXT("There is no cross-platform account provider configured."));
	}
}

void FAuthenticationGraphShib::Register()
{
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_SHIB,
        NSLOCTEXT("ShibMultiplayer", "AuthGraph_Shib", "Shib Authentication Graph"),
        MakeShared<FAuthenticationGraphShib>()
	);
}
}

#endif
