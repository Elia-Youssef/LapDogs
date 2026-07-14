// Copyright Shiba Inu Games LLC.

#pragma once

#include "UObject/Interface.h"
#include "ShibLoadingProcessInterface.generated.h"

/** Interface for things that might cause loading to happen which requires a loading screen to be displayed */
UINTERFACE(BlueprintType)
class SHIBASYNCLOADINGSCREEN_API UShibLoadingProcessInterface : public UInterface
{
	GENERATED_BODY()
};

class SHIBASYNCLOADINGSCREEN_API IShibLoadingProcessInterface
{
	GENERATED_BODY()
	
public:
	// Checks to see if this object implements the interface, and if so asks whether or not we should
	// be currently showing a loading screen
	static bool ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason);

	virtual bool ShouldShowLoadingScreen(FString& OutReason) const
	{
		return false;
	}
};
