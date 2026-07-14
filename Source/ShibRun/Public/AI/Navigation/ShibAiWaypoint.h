// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TargetPoint.h"
#include "ShibAiWaypoint.generated.h"

/**
 * 
 */
UCLASS()
class SHIBRUN_API AShibAiWaypoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TArray<AShibAiWaypoint*> NextWaypoints;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MainRouteOrder = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSplitHere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bMergeHere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BranchId = FName();
};
