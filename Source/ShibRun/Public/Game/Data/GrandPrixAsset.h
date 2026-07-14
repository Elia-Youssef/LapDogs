// Copyright Shiba Inu Games LLC.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GrandPrixAsset.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class SHIBRUN_API UGrandPrixAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=GrandPrixAsset)
	FSlateBrush GrandPrixThumbnail;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=GrandPrixAsset)
	FString GrandPrixId;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=GrandPrixAsset)
	FString GrandPrixDisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=GrandPrixAsset)
	bool RandomTracks=false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="!RandomTracks", EditConditionHides), Category=GrandPrixAsset)
	TArray<TSoftObjectPtr<UWorld>> GrandPrixTracks;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="RandomTracks", EditConditionHides, ClampMin=1), Category=GrandPrixAsset)
	int32 RaceRounds;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(EditCondition="RandomTracks", EditConditionHides), Category=GrandPrixAsset)
	TArray<TSoftObjectPtr<UWorld>> RandomTracksList;
};
