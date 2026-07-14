#include "Actors/VideoTriggerVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

AVideoTriggerVolume::AVideoTriggerVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	// Trigger volume component
	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	RootComponent = TriggerVolume;

	// Overlap
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &AVideoTriggerVolume::OnOverlapBegin);
}

void AVideoTriggerVolume::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor == UGameplayStatics::GetPlayerPawn(this, 0))
	{
		UGameplayStatics::SetGamePaused(this, true);

		// Create and display the video widget
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (PlayerController && VideoWidgetClass)
		{
			if (UUserWidget* VideoWidget = CreateWidget<UUserWidget>(PlayerController, VideoWidgetClass))
			{
				VideoWidget->AddToViewport();
				OnWidgetCreated(VideoWidget);
			}
		}
	}
}
