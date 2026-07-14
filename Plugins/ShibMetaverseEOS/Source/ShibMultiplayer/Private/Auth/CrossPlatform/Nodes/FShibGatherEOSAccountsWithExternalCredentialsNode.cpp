// Copyright Shiba Inu Games LLC.

#include "Auth/CrossPlatform/Nodes/FShibGatherEOSAccountsWithExternalCredentialsNode.h"
#include "Auth/CrossPlatform/ShibCrossPlatformAccountId.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"
#include "RedpointEOSAuth/ExternalCredentialType.h"
#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Auth::CrossPlatform
{
    using namespace ::Redpoint::EOS::API;

	void FShibGatherEOSAccountsWithExternalCredentialsNode::Execute(
		TSharedRef<FAuthenticationGraphState> State,
		FAuthenticationGraphNodeOnDone OnDone)
	{
    using namespace ::Redpoint::EOS::API::Connect;
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Compat;
    using namespace ::Redpoint::EOS::Auth::Nodes;
    using namespace ::Redpoint::EOS::Auth::Nodes;

    FMultiOperation<TSharedRef<IOnlineExternalCredentials>, bool>::RunSP(
        this,
        State->AvailableExternalCredentials,
        [this, State](
            const TSharedRef<IOnlineExternalCredentials> &ExternalCredentials,
            TFunction<void(bool)> OnPlatformDone) -> bool {
            FAuthenticationHelpersConnect::Login(
                State->PlatformHandle,
                ExternalCredentials->GetId(),
                ExternalCredentials->GetToken(),
                ExternalCredentials->GetUserDisplayName(),
                FExternalCredentialType::FromString(ExternalCredentials->GetType()),
                TDelegateCompat<FAuthenticationHelpersConnect::FLoginCompletionDelegate>::CreateSPLambda(
                    this,
                    [State,
                     ExternalCredentials,
                     OnPlatformDone](const FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                        if (bThisCallIsResponsibleForLogout)
                        {
                            State->AddLogoutNode(MakeShared<FSignOutEOSAccountNode>(Data.LocalUserId));
                        }
                        if (Data.ResultCode == EOS_EResult::EOS_Success ||
                            Data.ResultCode == EOS_EResult::EOS_InvalidUser)
                        {
                            State->AddEOSConnectCandidateFromExternalCredentials(
                                Data,
                                ExternalCredentials,
                                EAuthenticationGraphEOSCandidateType::CrossPlatform,
                                MakeShared<FShibCrossPlatformAccountId>(ExternalCredentials->GetId()));
                        }
                        else
                        {
                            State->ErrorMessages.Add(FString::Printf(
                                TEXT("ShibGatherEOSAccountsWithExternalCredentialsNode: External credential "
                                     "'%s' failed to authenticate with EOS Connect: %s"),
                                *ExternalCredentials->GetType(),
                                ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
                        }

                        OnPlatformDone(true);
                    }));
            return true;
        },
        [OnDone](const TArray<bool> &Results) {
            OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        });
	}
}
#endif