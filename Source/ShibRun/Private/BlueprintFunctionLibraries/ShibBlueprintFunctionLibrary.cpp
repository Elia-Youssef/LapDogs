// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintFunctionLibraries/ShibBlueprintFunctionLibrary.h"
#include "ShibRun/Public/Game/Data/StaticGameData.h"

EShibDirection4 UShibBlueprintFunctionLibrary::GetActorDirection4(const AActor* Actor, const FVector& Direction)
{
	const FVector2D Direction2D = FVector2D(Direction.X, Direction.Y).GetSafeNormal();

	if (!IsValid(Actor) || Direction2D.IsNearlyZero((0.01f)))
	{
		return EShibDirection4::Forward;
	}

	const FVector2d ActorFoward = FVector2d(Actor->GetActorForwardVector().X, Actor->GetActorForwardVector().Y).GetSafeNormal();
	const float ForwardDot = FVector2d::DotProduct(ActorFoward, Direction2D);

	if (FMath::Abs(ForwardDot) >= 0.5f)
	{
		if (ForwardDot >= 0.5f)
		{
			return EShibDirection4::Forward;
		}

		return EShibDirection4::Backward;
	}

	const FVector2d ActorRight = FVector2d(Actor->GetActorRightVector().X, Actor->GetActorRightVector().Y).GetSafeNormal();
	const float RightDot = FVector2d::DotProduct(ActorFoward, Direction2D);

	if (FMath::Abs(RightDot) >= 0.5f)
	{
		if (RightDot >= 0.5f)
		{
			return EShibDirection4::Right;
		}

		return EShibDirection4::Left;
	}
	
	return EShibDirection4::Forward;
}
