
#if EOS_HAS_AUTHENTICATION

#include "Auth/CrossPlatform/Nodes/ShibLoginToEosNode.h"
#include "Auth/PlatformIntegration/ShibExternalCredentials.h"
#include "Auth/CrossPlatform/ShibCrossPlatformAccountId.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"
#include "RedpointEOSAPI/Auth/CopyUserAuthToken.h"
#include "RedpointEOSCompat/DelegateCompat.h"

namespace Redpoint::EOS::Auth::CrossPlatform
{
	using namespace Redpoint::EOS::API::Auth;
	using namespace Redpoint::EOS::Platform::Integration;

	void FShibLoginToEosNode::Execute(
		TSharedRef<FAuthenticationGraphState> State,
		FAuthenticationGraphNodeOnDone OnDone)
	{
		using namespace Redpoint::EOS::API::Connect;

		if (!State->Metadata.Contains(TEXT("SHIB_ACCESS_TOKEN")))
		{
			State->ErrorMessages.Add(TEXT("Missing credentials to complete login"));
			OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
			return;
		}

		const FString& ShibAccessToken = State->Metadata[TEXT("SHIB_ACCESS_TOKEN")];
		const FString& ShibUserId = State->Metadata[TEXT("SHIB_USER_ID")];

		UE_LOG(LogTemp, Verbose, TEXT("Logging in to EOS..."));

		FAuthenticationHelpersConnect::Login(
			State->PlatformHandle,
			ShibUserId,
			ShibAccessToken,
			FString(),
			EOS_EExternalCredentialType::EOS_ECT_OPENID_ACCESS_TOKEN,
			Compat::TDelegateCompat<FAuthenticationHelpersConnect::FLoginCompletionDelegate>::CreateSPLambda(
				this,
				[this, State, OnDone, ShibUserId, ShibAccessToken](const FLogin::Result& Data, bool bThisCallIsResponsibleForLogout)
				{
					if (Data.ResultCode == EOS_EResult::EOS_Success || Data.ResultCode ==
						EOS_EResult::EOS_InvalidUser)
					{
						UE_LOG(LogTemp, Verbose, TEXT("Successfully logged in to EOS through Shib account"));

						FShibCredentialInfo ShibCredentialInfo;
						ShibCredentialInfo.UserId = ShibUserId;
						ShibCredentialInfo.SessionTicket = ShibAccessToken;
						ShibCredentialInfo.AuthAttributes.Add(TEXT("authenticatedWith"), TEXT("Shib"));
						ShibCredentialInfo.AuthAttributes.Add(TEXT("shib.UserId"), ShibUserId);
						ShibCredentialInfo.AuthAttributes.Add(TEXT("shib.Token"), ShibAccessToken);

						State->AddEOSConnectCandidateFromExternalCredentials(
							Data,
							MakeShared<FShibExternalCredentials>(ShibCredentialInfo),
							EAuthenticationGraphEOSCandidateType::CrossPlatform,
							MakeShared<FShibCrossPlatformAccountId>(ShibUserId)
						);

						OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
					}
					else if (Data.ResultCode == EOS_EResult::EOS_UnexpectedError)
					{
						// Epic's authentication API has intermittent issues with OpenID and returns EOS_UnexpectedError in
						// these cases. Just try again.
						UE_LOG(LogTemp, Warning, TEXT("Unexpected Error: Retrying..."));
						Execute(State, OnDone);
					}
					else
					{
						State->ErrorMessages.Add(FString::Printf(
							TEXT("Epic Account Services failed to authenticate with Shib. result code: %d"),
							Data.ResultCode));
						OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
					}
				}
			)
		);
	}
}

#endif
