// Copyright Shiba Inu Games LLC.

#include "Actors/RouteSpline.h"
#include "Components/SplineComponent.h"

ARouteSpline::ARouteSpline()
{
	PrimaryActorTick.bCanEverTick = false;

	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));

	// We always want the main road spline to be a closed loop spline that goes around the track
	Spline->SetClosedLoop(true);
}