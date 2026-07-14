
#if EOS_HAS_AUTHENTICATION

#include "Auth/CrossPlatform/ShibCrossPlatformAccountId.h"

namespace Redpoint::EOS::Auth::CrossPlatform
{

FShibCrossPlatformAccountId::FShibCrossPlatformAccountId(FString InShibAccountId) :
    DataBytes(nullptr),
    DataBytesSize(0),
    ShibAccountId(InShibAccountId)
{
    auto Str = StringCast<ANSICHAR>(*this->ToString());
    this->DataBytesSize = Str.Length() + 1;
    this->DataBytes = (uint8 *)FMemory::MallocZeroed(this->DataBytesSize);
    FMemory::Memcpy(this->DataBytes, Str.Get(), Str.Length());
}

bool FShibCrossPlatformAccountId::Compare(const FCrossPlatformAccountId &Other) const
{
    if (Other.GetType() != GetType())
    {
        return false;
    }

    if (Other.GetType() == SHIB_ACCOUNT_ID)
    {
        const FShibCrossPlatformAccountId &OtherId = (const FShibCrossPlatformAccountId &)Other;
        return OtherId.GetShibAccountId() == this->GetShibAccountId();
    }

    return (GetType() == Other.GetType() && GetSize() == Other.GetSize()) &&
           (FMemory::Memcmp(GetBytes(), Other.GetBytes(), GetSize()) == 0);
}

FShibCrossPlatformAccountId::~FShibCrossPlatformAccountId()
{
    FMemory::Free(this->DataBytes);
}

FName FShibCrossPlatformAccountId::GetType() const
{
    return SHIB_ACCOUNT_ID;
}

const uint8 *FShibCrossPlatformAccountId::GetBytes() const
{
    return this->DataBytes;
}

int32 FShibCrossPlatformAccountId::GetSize() const
{
    return this->DataBytesSize;
}

bool FShibCrossPlatformAccountId::IsValid() const
{
    return !ShibAccountId.IsEmpty();
}

FString FShibCrossPlatformAccountId::ToString() const
{
    return ShibAccountId;
}

TSharedPtr<const FCrossPlatformAccountId> FShibCrossPlatformAccountId::ParseFromString(const FString &In)
{
    return MakeShared<FShibCrossPlatformAccountId>(In);
}

FString FShibCrossPlatformAccountId::GetShibAccountId() const
{
    return ShibAccountId;
}
}

#endif
