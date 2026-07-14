#pragma once

#if EOS_HAS_AUTHENTICATION

#include "RedpointEOSAuth/Nodes/PromptToSignInOrCreateAccountNode.h"

namespace Redpoint::EOS::Auth::Nodes
{

/**
 * DEPRECATED
 * Kept as reference
 */
class SHIBMULTIPLAYER_API FShibPromptToSignInNode : public FPromptToSignInOrCreateAccountNode
{
private:
    virtual void SelectChoice(
        EEOSUserInterface_SignInOrCreateAccount_Choice SelectedChoice,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone) override;

public:
    virtual FString GetDebugName() const override
    {
        return TEXT("FShibPromptToSignInNode");
    }
};

}

#endif
