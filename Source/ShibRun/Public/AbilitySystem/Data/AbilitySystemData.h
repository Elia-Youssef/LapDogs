// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemData.generated.h"

USTRUCT(BlueprintType)
struct FTraceParams
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trace")
	bool bTraceComplex = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trace")
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trace")
	FColor DebugColorOnHit = FColor::Green;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trace")
	FColor DebugColorOnFail = FColor::Red;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Trace")
	float DebugDuration = 5.0f;
};

UENUM(BlueprintType)
enum class ETargetHitType : uint8
{
	None,
	Static,
	Dynamic,
	Pawn
};

USTRUCT(BlueprintType)
struct FHitscanData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	AActor* HitActor = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	AActor* SourceActor = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	UPrimitiveComponent* HitComponent = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	FVector RelativeHitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	FVector SurfaceNormal = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	ETargetHitType TargetHitType = ETargetHitType::None;

	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	FName BoneName = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	float Radius = 0.0f;
};

USTRUCT(BlueprintType)
struct FProjectileData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	FVector RelativeHitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	FVector SurfaceNormal = FVector::ZeroVector;

	// This is the local spawn point of the shooter, typically the Muzzle socket of the equipped weapo
	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	FVector SpawnLocation = FVector::ZeroVector;

	// This is the aim the projectile was fired in.
	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	FRotator LaunchRotation = FRotator::ZeroRotator;

	// This is the direction the projectile was fired in.
	UPROPERTY(BlueprintReadWrite, Category = "Hitscan Data")
	FVector LaunchDirection = FVector::ZeroVector;
};