// Copyright Shiba Inu Games LLC.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShibObstacle.generated.h"

class AShibObstacleManager;

UCLASS()
class SHIBRUN_API AShibObstacle : public AActor
{
	GENERATED_BODY()
public:

	virtual void BeginPlay() override;

	void StateChanged(bool bNewState);

	UFUNCTION(BlueprintImplementableEvent)
	void OnStateChanged(bool bNewState);
	
};
