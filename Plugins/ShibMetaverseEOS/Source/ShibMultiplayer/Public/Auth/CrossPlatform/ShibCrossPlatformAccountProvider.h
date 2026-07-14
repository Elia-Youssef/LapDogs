// Copyright Shiba Inu Games LLC.
#pragma once

#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountProvider.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Auth::CrossPlatform
{

class SHIBMULTIPLAYER_API FShibCrossPlatformAccountProvider : public ICrossPlatformAccountProvider
{
public:
    FShibCrossPlatformAccountProvider(){};

    virtual FName GetName() const override;
    virtual TSharedPtr<const FCrossPlatformAccountId> CreateCrossPlatformAccountId(const FString &InStringRepresentation) const override;
    virtual TSharedPtr<const FCrossPlatformAccountId> CreateCrossPlatformAccountId(uint8 *InBytes, int32 InSize) const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetInteractiveAuthenticationSequence() const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetInteractiveOnlyAuthenticationSequence() const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetNonInteractiveAuthenticationSequence(bool bOnlyUseExternalCredentials) const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetUpgradeCurrentAccountToCrossPlatformAccountSequence() const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetLinkUnusedExternalCredentialsToCrossPlatformAccountSequence() const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetNonInteractiveDeauthenticationSequence() const override;
    virtual TSharedRef<FAuthenticationGraphNode> GetAutomatedTestingAuthenticationSequence() const override;

private:
    bool IsUserLoggedIn();
};

}
#endif