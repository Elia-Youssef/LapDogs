#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Volume.h"
#include "Runtime/MediaAssets/Public/MediaSource.h"
#include "VideoTriggerVolume.generated.h"

UCLASS()
class SHIBRUN_API AVideoTriggerVolume : public AActor
{
	GENERATED_BODY()

public:
	AVideoTriggerVolume();

	// Widget class to be set in the editor
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> VideoWidgetClass;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "UI")
	FString Headline;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "UI")
	FString Description;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "UI")
	TArray<FString> Hotkeys;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UMediaSource> Video;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UTexture2D> Image;

	UFUNCTION(BlueprintImplementableEvent)
	void OnWidgetCreated(UUserWidget* VideoWidget );

private:
	UPROPERTY(EditAnywhere, Category = "Components")
	UBoxComponent* TriggerVolume;
	
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
