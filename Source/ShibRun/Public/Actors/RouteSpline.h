// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RouteSpline.generated.h"

class USplineComponent;

UCLASS()
class SHIBRUN_API ARouteSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	ARouteSpline();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USplineComponent> Spline;
};
