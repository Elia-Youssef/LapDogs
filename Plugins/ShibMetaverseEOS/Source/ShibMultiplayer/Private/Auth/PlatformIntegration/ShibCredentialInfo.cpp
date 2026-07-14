// Copyright Shiba Inu Games LLC.

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

#include "Auth/PlatformIntegration/ShibCredentialInfo.h"

namespace Redpoint::EOS::Platform::Integration
{
	FShibCredentialInfo::FShibCredentialInfo()
	: UserId(TEXT(""))
	, SessionTicket(TEXT(""))
	{
	
	}
}

#endif