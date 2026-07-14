// Copyright Shiba Inu Games LLC.

#include "Utils/ShibStatics.h"

float UShibStatics::GetDateTimeDifference(const FDateTime Start, const FDateTime End)
{
	return (End - Start).GetTotalSeconds();
}

FString UShibStatics::GetTimeFromSeconds(const int32& Seconds)
{
	return FString::Printf(TEXT("%d:%d"), Seconds / 60, Seconds % 60);
}

bool UShibStatics::RunningInPIE()
{
#if WITH_EDITOR
	return true;
#else
	return false;
#endif
}
