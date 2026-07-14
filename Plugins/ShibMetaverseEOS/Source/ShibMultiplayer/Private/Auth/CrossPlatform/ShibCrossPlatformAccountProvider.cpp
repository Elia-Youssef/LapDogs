// Copyright Shiba Inu Games LLC.

#include "Auth/CrossPlatform/ShibCrossPlatformAccountProvider.h"
#include "Auth/CrossPlatform/Nodes/FShibGatherEOSAccountsWithExternalCredentialsNode.h"
#include "RedpointEOSAuth/Nodes/BailIfNotExactlyOneExternalCredentialNode.h"
#include "RedpointEOSAuth/Nodes/GatherEOSAccountsWithExternalCredentialsNode.h"
#include "Auth/Graphs/AuthenticationGraphShib.h"
#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/SelectCrossPlatformAccountNode.h"
#include "Auth/CrossPlatform/Nodes/ShibGetExternalCredentialsNode.h"
#include "Auth/CrossPlatform/ShibCrossPlatformAccountId.h"
#include "RedpointEOSAuth/Nodes/NoopAuthenticationGraphNode.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "Auth/CrossPlatform/Nodes/ShibGetCommandLineTokenNode.h"
#include "Auth/CrossPlatform/Nodes/ShibLoginToEosNode.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
namespace Redpoint::EOS::Auth::CrossPlatform
{
using namespace Redpoint::EOS::Auth::Nodes;
using namespace Redpoint::EOS::Platform::Integration;

FName FShibCrossPlatformAccountProvider::GetName() const
{
    return SHIB_ACCOUNT_ID;
}

TSharedPtr<const FCrossPlatformAccountId> FShibCrossPlatformAccountProvider::CreateCrossPlatformAccountId(const FString &InStringRepresentation) const
{
    return FShibCrossPlatformAccountId::ParseFromString(InStringRepresentation);
}

TSharedPtr<const FCrossPlatformAccountId> FShibCrossPlatformAccountProvider::CreateCrossPlatformAccountId(uint8 *InBytes, int32 InSize) const
{
    FString Data = BytesToString(InBytes, InSize);
    return FShibCrossPlatformAccountId::ParseFromString(Data);
}

TSharedRef<FAuthenticationGraphNode> FShibCrossPlatformAccountProvider::GetInteractiveAuthenticationSequence() const
{
    /*
   return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
       ->Add(MakeShared<FShibGetCommandLineTokenNode>())
       ->Add(MakeShared<FShibLoginToEosNode>());
   */
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FShibCrossPlatformAccountProvider::GetInteractiveOnlyAuthenticationSequence() const
{
    /*
   return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
       ->Add(MakeShared<FShibGetCommandLineTokenNode>())
       ->Add(MakeShared<FShibLoginToEosNode>());
   */
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FShibCrossPlatformAccountProvider::GetNonInteractiveAuthenticationSequence(bool bOnlyUseExternalCredentials) const
{
    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FShibGetExternalCredentialsNode>())
        ->Add(MakeShared<FBailIfNotExactlyOneExternalCredentialNode>())
        ->Add(MakeShared<FShibGatherEOSAccountsWithExternalCredentialsNode>())
        ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
        ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>());
}

TSharedRef<FAuthenticationGraphNode> FShibCrossPlatformAccountProvider::GetUpgradeCurrentAccountToCrossPlatformAccountSequence() const
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FShibCrossPlatformAccountProvider::GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence() const
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FShibCrossPlatformAccountProvider::GetNonInteractiveDeauthenticationSequence() const
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}

TSharedRef<FAuthenticationGraphNode> FShibCrossPlatformAccountProvider::GetAutomatedTestingAuthenticationSequence() const
{
    return MakeShared<FNoopAuthenticationGraphNode>();
}
    
}

#endif
