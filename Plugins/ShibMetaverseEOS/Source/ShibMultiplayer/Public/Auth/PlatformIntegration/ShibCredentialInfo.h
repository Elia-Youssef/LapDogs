// Copyright Shiba Inu Games LLC.

#pragma once

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace Redpoint::EOS::Platform::Integration
{

struct FShibCredentialInfo
{
public:
	FShibCredentialInfo();

	FString UserId;
	FString SessionTicket;
	TMap<FString, FString> AuthAttributes;
};

}
#endif