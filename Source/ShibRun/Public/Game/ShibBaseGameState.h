// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "ShibBaseGameState.generated.h"

class AShibRaceManager;
class AShibGameMode;

/**
 * 
 */
UCLASS()
class SHIBRUN_API AShibBaseGameState : public AGameState
{
	GENERATED_BODY()
	
public:
	// temp
	FName GetShibMatchState() const { return GetMatchState(); }

protected:
	UPROPERTY()
	TObjectPtr<AShibGameMode> ShibGM;
	TObjectPtr<AShibGameMode> GetShibGM();
};
