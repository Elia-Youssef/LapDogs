// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "ShibGameSession.generated.h"

UCLASS()
class SHIBRUN_API AShibGameSession : public AGameSession
{
	GENERATED_BODY()

public:

	// Kick player from the current session
	// We don't call the parent function here because the implementation is bad and unfinished in the engine source code
	virtual bool KickPlayer(APlayerController* KickedPlayer, const FText& KickReason) override;
};
