// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StaticGameData.generated.h"

UENUM(BlueprintType)
enum class EShibDirection4 : uint8
{
	Forward				UMETA(DisplayName = "Forward"),
	Left				UMETA(DisplayName = "Left"),
	Right				UMETA(DisplayName = "Right"),
	Backward			UMETA(DisplayName = "Backward")
};