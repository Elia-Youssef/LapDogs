// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FixedSpectatorCamera.generated.h"

class UCameraComponent;

UCLASS()
class SHIBRUN_API AFixedSpectatorCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	AFixedSpectatorCamera();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCameraComponent> Camera;
};
