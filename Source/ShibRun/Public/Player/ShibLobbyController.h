// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBaseController.h"
#include "ShibLobbyController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceStarting, bool, bRaceIsStarting);

/**
 * 
 */
UCLASS()
class SHIBRUN_API AShibLobbyController : public AShibBaseController
{
	GENERATED_BODY()
	
};
