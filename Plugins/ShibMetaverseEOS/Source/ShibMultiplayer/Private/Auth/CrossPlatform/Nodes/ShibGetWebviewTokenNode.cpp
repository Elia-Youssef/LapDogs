
#if EOS_HAS_AUTHENTICATION

#include "Auth/CrossPlatform/Nodes/ShibGetWebviewTokenNode.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "ShibUserEOS.h"
#include "Auth/AuthenticationHelpersShib.h"

namespace Redpoint::EOS::Auth::CrossPlatform
{

void FShibGetWebviewTokenNode::Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone)
{
    if (State->GetWorld() && State->GetWorld()->GetGameInstance())
    {
        if (auto* ShibUserEOS = State->GetWorld()->GetGameInstance()->GetSubsystem<UShibUserEOS>())
        {
            // USES LoginData from ShibUserEOS, which is set in blueprints (now deleted)
            
            // if (FAuthenticationHelpersShib::ParseLoginDataFromJSON(ShibUserEOS->LoginData, State)) {
            //     OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            //     return;
            // }

            OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
            return;
        }
    }
 
    State->ErrorMessages.Add(FString(TEXT("Invalid world or game instance.")));
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
}

}

#endif
