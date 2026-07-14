
#if EOS_HAS_AUTHENTICATION

#include "Auth/CrossPlatform/Nodes/ShibPromptToSignInNode.h"

namespace Redpoint::EOS::Auth::Nodes
{
	
void FShibPromptToSignInNode::SelectChoice(
	EEOSUserInterface_SignInOrCreateAccount_Choice SelectedChoice, TSharedRef<FAuthenticationGraphState> State,
	FAuthenticationGraphNodeOnDone OnDone)
{
    State->LastSignInChoice = SelectedChoice;
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

}

#endif
