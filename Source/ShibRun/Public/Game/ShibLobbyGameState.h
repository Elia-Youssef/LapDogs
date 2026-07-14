// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "ShibBaseGameState.h"
#include "ShibLobbyGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerArrayModifiedDelegate);

/**
 * 
 */
UCLASS()
class SHIBRUN_API AShibLobbyGameState : public AShibBaseGameState
{
	GENERATED_BODY()

	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;

	UPROPERTY(BlueprintAssignable)
	FPlayerArrayModifiedDelegate PlayerArrayModified;
};
