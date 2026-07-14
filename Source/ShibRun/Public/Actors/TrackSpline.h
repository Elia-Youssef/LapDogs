// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackSpline.generated.h"

class USplineComponent;

UCLASS()
class SHIBRUN_API ATrackSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	ATrackSpline();
	
	virtual void BeginPlay() override;
	
	void GenerateSplinePoints() const;

	bool HasSurface(const FVector& Location) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USplineComponent> Spline;

	/**If set to 0, will not spawn any points.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceBetweenPoints = 1000.f;

	/**Each point will check if there's a surface beneath it by this range. If set to 0, will not check for surfaces.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceRangeCheck = 1000.f;
};
