#pragma once

#if EOS_HAS_AUTHENTICATION

#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"

namespace Redpoint::EOS::Auth::CrossPlatform
{
#define SHIB_ACCOUNT_ID FName(TEXT("Shib"))

class SHIBMULTIPLAYER_API FShibCrossPlatformAccountId : public FCrossPlatformAccountId
{
    uint8 *DataBytes;
    int32 DataBytesSize;
    FString ShibAccountId;
    
public:
    FShibCrossPlatformAccountId(FString InShibAccountId);
    virtual ~FShibCrossPlatformAccountId();
    UE_NONCOPYABLE(FShibCrossPlatformAccountId);

    virtual bool Compare(const FCrossPlatformAccountId &Other) const override;
    virtual FName GetType() const override;
    virtual const uint8 *GetBytes() const override;
    virtual int32 GetSize() const override;
    virtual bool IsValid() const override;
    virtual FString ToString() const override;

    static TSharedPtr<const FCrossPlatformAccountId> ParseFromString(const FString &In);

    FString GetShibAccountId() const;
};
 
}

#endif
