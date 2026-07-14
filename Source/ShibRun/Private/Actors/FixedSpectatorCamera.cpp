// Copyright Shiba Inu Games LLC.


#include "Actors/FixedSpectatorCamera.h"

#include "Camera/CameraComponent.h"

AFixedSpectatorCamera::AFixedSpectatorCamera()
{
	PrimaryActorTick.bCanEverTick = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	SetRootComponent(Camera);
}