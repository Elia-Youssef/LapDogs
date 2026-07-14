// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ShibStatics.generated.h"

/**
 * Static functions
 */
UCLASS()
class SHIBRUN_API UShibStatics : public UObject
{
	GENERATED_BODY()

public:
	// Get time difference in seconds
	UFUNCTION(BlueprintCallable, Category = "ShibStatics")
	static float GetDateTimeDifference(FDateTime Start, FDateTime End);

	// Get time string (00:00) given seconds
	UFUNCTION(BlueprintCallable, Category = "ShibStatics")
	static FString GetTimeFromSeconds(const int32& Seconds);
	
	// Allow the blueprint to determine whether we are running with the editor or not
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "ShibStatics")
	static bool RunningInPIE();
};
