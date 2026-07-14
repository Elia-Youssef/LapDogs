// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShibCheckpoint.generated.h"

UCLASS()
class SHIBRUN_API AShibCheckpoint : public AActor
{
	GENERATED_BODY()
	
public:	

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName CheckpointID = "0";
};
