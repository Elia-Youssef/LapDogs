// Copyright Shiba Inu Games LLC.

#include "Actors/TrackSpline.h"
#include "Components/SplineComponent.h"

ATrackSpline::ATrackSpline()
{
	PrimaryActorTick.bCanEverTick = false;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
}

void ATrackSpline::BeginPlay()
{
	Super::BeginPlay();
	
	GenerateSplinePoints();
}

void ATrackSpline::GenerateSplinePoints() const
{
	if (!Spline || DistanceBetweenPoints <= 0.f)
	{
		return;
	}

	const float SplineLength = Spline->GetSplineLength();
	const int32 NumPoints = FMath::CeilToInt(SplineLength / DistanceBetweenPoints);

	// We don't create the points immediately. We store their locations so we can clear the pre existing points first
	TArray<FVector> NewPoints;

	for (int32 i = 0; i <= NumPoints; ++i)
	{
		float DistanceAlongSpline = i * DistanceBetweenPoints;
		// Clamp the distance to the spline length to avoid out of bounds errors
		DistanceAlongSpline = FMath::Clamp(DistanceAlongSpline, 0.0f, SplineLength);
		FVector LocalPoint = Spline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::Local);

		if (HasSurface(Spline->GetLocationAtDistanceAlongSpline(DistanceAlongSpline, ESplineCoordinateSpace::World))) // Checking for surfaces based on World instead of Local
		{
			NewPoints.Add(LocalPoint); // Storing location for valid point
		}
	}

	// This will clear all spline points that were placed in the editor
	Spline->ClearSplinePoints(true);

	// Now we create the stored points to the spline
	for (const FVector& Point : NewPoints)
	{
		Spline->AddSplinePoint(Point, ESplineCoordinateSpace::Local, true);
	}

	Spline->UpdateSpline();
}

bool ATrackSpline::HasSurface(const FVector& Location) const
{
	if(SurfaceRangeCheck <= 0 || Location.IsZero())
	{
		return false;
	}
	
	const FVector Start = Location;
	const FVector End = Start - FVector(0,0, SurfaceRangeCheck);
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
	
	return HitResult.bBlockingHit;
}
