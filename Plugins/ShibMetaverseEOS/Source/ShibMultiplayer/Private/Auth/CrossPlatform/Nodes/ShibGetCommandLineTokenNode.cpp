
#if EOS_HAS_AUTHENTICATION

#include "Auth/CrossPlatform/Nodes/ShibGetCommandLineTokenNode.h"

#include "Interfaces/IHttpResponse.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "ShibMultiplayerSettings.h"
#include "Auth/AuthenticationHelpersShib.h"

namespace Redpoint::EOS::Auth::CrossPlatform
{

void FShibGetCommandLineTokenNode::Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone)
{
    auto* Settings = GetMutableDefault<UShibMultiplayerSettings>();
    if (!Settings || Settings->TokenArgKey.IsEmpty() || Settings->UserIdArgKey.IsEmpty())
    {
        State->ErrorMessages.Add(TEXT("Invalid token or user data argument key. Check your project settings."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    if (FString Arg; FParse::Value(FCommandLine::Get(), *FString::Printf(TEXT("%s="), *Settings->TokenArgKey), Arg))
    {
        UE_LOG(LogTemp, Verbose, TEXT("%s: %s"), *Settings->TokenArgKey, *Arg);
        State->Metadata.Add(TEXT("SHIB_ACCESS_TOKEN"), Arg);
    } else
    {
        State->ErrorMessages.Add(FString::Printf(TEXT("Invalid token argument for key `%s`."), *Arg));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }
    
    if (FString Arg; FParse::Value(FCommandLine::Get(), *FString::Printf(TEXT("%s="), *Settings->UserIdArgKey), Arg))
    {
        UE_LOG(LogTemp, Verbose, TEXT("%s: %s"), *Settings->UserIdArgKey, *Arg);
        State->Metadata.Add(TEXT("SHIB_USER_ID"), Arg);
    } else
    {
        State->Metadata.Add(TEXT("SHIB_USER_ID"), FString());
        // State->ErrorMessages.Add(FString::Printf(TEXT("Invalid user data argument for key `%s`."), *Arg));
        // OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        // return;
    }
    
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

}

#endif
