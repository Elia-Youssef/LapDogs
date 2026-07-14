// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IceSpline.generated.h"

class USplineComponent;

UCLASS()
class SHIBRUN_API AIceSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	AIceSpline();
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	void UpdateSpline() const;

	void GenerateSplinePoints() const;

	FVector GetSurface(const FVector& Location) const;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ActorToSpawn;

	/**If set to 0, will not spawn any points.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceBetweenPoints = 50.f;

	/**Each point will check if there's a surface beneath it by this range. If set to 0, will not check for surfaces.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SurfaceRangeCheck = 200.f;
};
