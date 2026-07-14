// Copyright Shiba Inu Games LLC.

#include "Actors/IceSpline.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Character.h"

AIceSpline::AIceSpline()
{
	PrimaryActorTick.bCanEverTick = true;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
}

void AIceSpline::BeginPlay()
{
	Super::BeginPlay();

	if(ActorToSpawn == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Ice Spline does not have a set actor class to spawn."));
		Destroy();
		return;
	}

	if(GetInstigator())
	{
		if(const ACharacter* Character = Cast<ACharacter>(GetOwner()))
		{
			const FVector CharacterLocation = Character->GetActorLocation();
			Spline->AddSplinePoint(CharacterLocation, ESplineCoordinateSpace::World, true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Ice Spline does not have character Owner."));
			Destroy();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Ice Spline does not have instigator."));
		Destroy();
	}

}

void AIceSpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateSpline();
}

void AIceSpline::UpdateSpline() const
{
	// This function a little 'special' because it checks for missing "holes" in case there's bad movement update or low fps
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!Character || !Spline || DistanceBetweenPoints <= 0.0f)
	{
		return;
	}

	const FVector CharacterLocation = Character->GetActorLocation();
	FVector LastSplinePoint = Spline->GetLocationAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);

	// Calculate the total distance to cover
	float DistanceToCover = FVector::Dist(CharacterLocation, LastSplinePoint);

	// Loop to cover the entire distance with points spaced at DistanceBetweenPoints
	while (DistanceToCover >= DistanceBetweenPoints)
	{
		// Move along the spline by DistanceBetweenPoints
		FVector Direction = (CharacterLocation - LastSplinePoint).GetSafeNormal();
		LastSplinePoint += Direction * DistanceBetweenPoints;

		FVector SurfaceLocation = GetSurface(LastSplinePoint);
		
		// Check for surface and add spline point
		if (SurfaceLocation != FVector::ZeroVector)
		{
			Spline->AddSplinePoint(LastSplinePoint, ESplineCoordinateSpace::World, true);
			Spline->UpdateSpline();

			// Spawn the actor at the new spline point
			FActorSpawnParameters SpawnParameters;
			SpawnParameters.Instigator = GetInstigator();
			SpawnParameters.Owner = GetOwner();

			GetWorld()->SpawnActor<AActor>(ActorToSpawn, SurfaceLocation, FRotator::ZeroRotator, SpawnParameters);
		}

		// Recalculate the remaining distance to cover
		DistanceToCover = FVector::Dist(CharacterLocation, LastSplinePoint);
	}
}

FVector AIceSpline::GetSurface(const FVector& Location) const
{
	if(SurfaceRangeCheck <= 0 || Location.IsZero())
	{
		return FVector::ZeroVector;
	}
	
	const FVector Start = Location;
	const FVector End = Start - FVector(0,0, SurfaceRangeCheck);
	
	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
	
	return HitResult.Location;
}
